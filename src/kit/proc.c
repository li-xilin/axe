#include "ax/proc.h"
#include "ax/sys.h"
#include "ax/detect.h"
#include "ax/errno.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#ifdef AX_OS_WIN

#include <sys/fcntl.h>
#include <io.h>
#include <tchar.h>
#include <windows.h>

#include <stdio.h>

struct ax_proc_st
{
	pid_t pid;
	HANDLE hProcess;
	FILE *in, *out, *err;
};

#define POPEN_PIPE_BUFFER_SIZE 4096
#define popen3_fatal(x)  _ftprintf(stderr, _T(x)); return -1;

FILE* fhopen(HANDLE hFile, const char *zMode)
{
    int fd = _open_osfhandle((intptr_t)hFile, _O_BINARY);
    if( fd != -1)
        return _fdopen(fd, zMode);
    else
        return NULL;
}

static HANDLE SpawnChildWithHandle(LPCWSTR pCmdLine, HANDLE hIn, HANDLE hOut, HANDLE hErr, LPDWORD pProcessID)
{
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESS_INFORMATION pi = { 0 };

	LPWSTR pCmdBuf = HeapAlloc(GetProcessHeap(), 0, 32 * 1024);
	wcscpy(pCmdBuf, pCmdLine);

	STARTUPINFOW si = { 
		.cb = sizeof (STARTUPINFO),
		.dwFlags = STARTF_USESTDHANDLES,
		.hStdInput  = hIn,
		.hStdOutput = hOut,
		.hStdError  = hErr,
	};

	if(!SetHandleInformation(hIn, HANDLE_FLAG_INHERIT, TRUE)) {
		ax_error_occur();
		goto out;
	}

	if(!SetHandleInformation(hOut, HANDLE_FLAG_INHERIT, TRUE)) {
		ax_error_occur();
		goto out;
	}

	if(!SetHandleInformation(hErr, HANDLE_FLAG_INHERIT, TRUE)) {
		ax_error_occur();
		goto out;
	}

	if (!CreateProcessW(NULL, pCmdBuf, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		ax_error_occur();
		goto out;
	}


	CloseHandle(pi.hProcess);
	*pProcessID = pi.dwProcessId;
	hProcess = pi.hProcess;
out:
	return hProcess;
}

static BOOL SpawnChild(LPCWSTR pCmdLine, struct ax_proc_st *proc)
{
	BOOL bSuccess = FALSE;
	HANDLE hInput[2] = { 0 }, hOutput[2] = { 0 }, hError[2] = { 0 };
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	SECURITY_ATTRIBUTES saAttr;

	proc->in = proc->out = proc->err = NULL;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE; /* Set the bInheritHandle flag so pipe handles are inherited. */
	saAttr.lpSecurityDescriptor = NULL; 

	if (!CreatePipe(&hInput[0], &hInput[1], &saAttr, POPEN_PIPE_BUFFER_SIZE)) {
		ax_error_occur();
		goto out;
	}

	if (!CreatePipe(&hOutput[1], &hOutput[1], &saAttr, POPEN_PIPE_BUFFER_SIZE)) {
		ax_error_occur();
		goto out;
	}

	if (!CreatePipe(&hError[0], &hError[1], &saAttr, POPEN_PIPE_BUFFER_SIZE)) {
		ax_error_occur();
		goto out;
	}

	if (!SetHandleInformation(hInput[1], HANDLE_FLAG_INHERIT, FALSE)
			|| !SetHandleInformation(hOutput[1], HANDLE_FLAG_INHERIT, FALSE)
			|| !SetHandleInformation(hError[0], HANDLE_FLAG_INHERIT, FALSE)) {
		ax_error_occur();
		goto out;
	}

	DWORD dwProcessId;
	if ((hProcess = SpawnChildWithHandle(pCmdLine, hInput[0], hOutput[1],
					hError[1], &dwProcessId)) == INVALID_HANDLE_VALUE)
		goto out;

	proc->in = fhopen(hInput[1], "wb");
	proc->out = fhopen(hOutput[0], "rb");
	proc->err = fhopen(hError[0], "rb");
	if (!proc->in || !proc->out || !proc->err)
		goto out;
	proc->hProcess = hProcess;
	proc->pid = dwProcessId;

	bSuccess = TRUE;
out:
	if (!bSuccess) {
		if (proc->in)
			fclose(proc->in);
		else if (hInput[1])
			CloseHandle(hInput[1]);
		if (proc->out)
			fclose(proc->out);
		else if (hOutput[0])
			CloseHandle(hOutput[0]);
		if (proc->err)
			fclose(proc->err);
		else if (hError[0])
			CloseHandle(hError[0]);
	}

	if (hInput[0])
		CloseHandle(hInput[0]); 
	if (hOutput[1])
		CloseHandle(hOutput[1]);
	if (hError[1])
		CloseHandle(hError[1]);
	return bSuccess;
}

ax_proc *ax_proc_open(const ax_uchar* cmdline)
{
	ax_proc *proc = malloc(sizeof(*proc));
	if (!proc)
		return NULL;

	if (SpawnChild(cmdline, proc)) {
		free(proc);
		return NULL;
	}

	return proc;
}

int ax_proc_kill(const ax_proc *proc)
{
	if (!proc) {
		errno = EINVAL;
		return -1;
	}
	if (!TerminateProcess(proc->hProcess, 0)) {
		ax_error_occur();
		return -1;
	}
	return 0;
}

int ax_proc_close(ax_proc *proc)
{
	int exit_code = -1;
	if (!proc) {
		errno = AX_EINVAL;
		goto out;
	}

	fclose(proc->in);
	fclose(proc->out);

	if (WaitForSingleObject(proc->hProcess, INFINITE)) {
		ax_error_occur();
		goto out;
	}
	DWORD dwExitCode;
	if (GetExitCodeProcess(proc->hProcess, &dwExitCode))
		goto out;
	exit_code = dwExitCode;
out:
	CloseHandle(proc->hProcess);
	free(proc);
	return exit_code;
}

#else

#include <sys/wait.h>
#include <unistd.h>
#include <unistd.h>

#define CLOSE_PIPE(pipe) close((pipe)[0]); close((pipe)[1])

/* To close all fd duplications after forked */
static ax_proc *sg_proc_chain;

struct ax_proc_st
{
	struct ax_proc_st *next;
	pid_t pid;
	FILE *in, *out, *err;
};

static int _do_popen3(ax_proc *proc, const char *command)
{
	int retval = -1;
	int child_in[2] = { -1 };
	int child_out[2] = { -1 };
	int child_err[2] = { -1 };

	if (pipe(child_in) || pipe(child_out) || pipe(child_out)) {
		ax_error_occur();
		goto out;
	}

	pid_t cpid = fork();
	if (cpid < 0) {
		ax_error_occur();
		goto out;
	}
	if (cpid == 0) {
		if (0 > dup2(child_in[0], 0) || 0 > dup2(child_out[1], 1)) {
			_Exit(127);
		}
		CLOSE_PIPE(child_in);
		CLOSE_PIPE(child_out);

		for (ax_proc *p = sg_proc_chain; p; p = p->next) {
			close(fileno(p->in));
			close(fileno(p->out));
			close(fileno(p->err));
		}

		execl("/bin/sh", "sh", "-c", command, (char *) NULL);
		_Exit(127);
	}

	close(child_in[0]);
	close(child_out[1]);
	close(child_err[1]);
	proc->in = fdopen(child_in[1], "w");
	proc->out = fdopen(child_out[0], "r");
	proc->err = fdopen(child_err[0], "r");
	proc->pid = cpid;

	retval = 0;
out:
	if (retval) {
		if (child_in[0] != -1)
			CLOSE_PIPE(child_in);
		if (child_out[0] != -1)
			CLOSE_PIPE(child_out);
		if (child_err[0] != -1)
			CLOSE_PIPE(child_err);
	}
	return retval;
}

ax_proc *ax_proc_open(const ax_uchar* cmdline)
{
	ax_proc *proc = malloc(sizeof(*proc));
	if (!proc)
		return NULL;

	if (0 > _do_popen3(proc, cmdline)) {
		free(proc);
		return NULL;
	}

	proc->next = sg_proc_chain;
	sg_proc_chain = proc;

	return proc;
}

int ax_proc_kill(const ax_proc *proc)
{
	int retval;
	if ((retval = kill(proc->pid, SIGKILL))) {
		ax_error_occur();
	}
	return retval;
}

int ax_proc_close(ax_proc *proc)
{
	ax_proc **p = &sg_proc_chain;
	while (*p) {
		if (*p == proc) {
			*p = (*p)->next;
			goto found;
		}
		p = &(*p)->next;
	}
	errno = EINVAL;
	return -1;

found:
	if (0 > fclose(proc->in) || 0 > fclose(proc->out)) {
		free(proc);
		return -1;
	}

	int status = -1;
	pid_t wait_pid;
	do {
		wait_pid = waitpid(proc->pid, &status, 0);
	} while (-1 == wait_pid && EINTR == errno);

	free(proc);

	if (wait_pid == -1) {
		ax_error_occur();
		return -1;
	}
	return status;
}
#endif

FILE *ax_proc_stdin(const ax_proc *proc)
{
	return proc->in;
}

FILE *ax_proc_stdout(const ax_proc *proc)
{
	return proc->out;
}

FILE *ax_proc_stderr(const ax_proc *proc)
{
	return proc->err;
}

pid_t ax_proc_pid(const ax_proc *proc)
{
	return proc->pid;
}

