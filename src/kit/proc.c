/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "ax/proc.h"
#include "ax/sys.h"
#include "ax/detect.h"
#include "ax/errno.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#ifdef AX_OS_WIN

#include <ax/mem.h>
#include <io.h>
#include <sys/fcntl.h>
#include <tchar.h>
#include <windows.h>
#include <stdio.h>

struct ax_proc_st
{
	pid_t pid;
	HANDLE hProcess;
	FILE *in, *out, *err;
};

#define PIPE_BUFSIZ 4096

static FILE* fhopen(HANDLE hFile, const char *zMode)
{
    int fd = _open_osfhandle((intptr_t)hFile, _O_BINARY);
    if( fd != -1)
        return _fdopen(fd, zMode);
    else
        return NULL;
}

static HANDLE CreateChildProcess(LPCWSTR pFile, LPWSTR pCmdLine, HANDLE hIn, HANDLE hOut, HANDLE hErr, LPDWORD pProcessID)
{
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	PROCESS_INFORMATION pi = { 0 };

	STARTUPINFOW si = { 
		.cb = sizeof si,
		.dwFlags = STARTF_USESTDHANDLES,
		.hStdInput  = hIn,
		.hStdOutput = hOut,
		.hStdError  = hErr,
	};

	if (!CreateProcessW(pFile, pCmdLine, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
		ax_error_occur();
		goto out;
	}

	CloseHandle(pi.hThread);
	*pProcessID = pi.dwProcessId;
	hProcess = pi.hProcess;
out:
	return hProcess;
}

static BOOL ExecuteWithPipe(LPCWSTR pwzFile, LPWSTR pwzCmdLine, struct ax_proc_st *proc)
{
	BOOL bSuccess = FALSE;
	HANDLE hInput[2] = { 0 }, hOutput[2] = { 0 }, hError[2] = { 0 };
	HANDLE hProcess = INVALID_HANDLE_VALUE;
	SECURITY_ATTRIBUTES saAttr;

	proc->in = proc->out = proc->err = NULL;

	saAttr.nLength = sizeof(SECURITY_ATTRIBUTES); 
	saAttr.bInheritHandle = TRUE;
	saAttr.lpSecurityDescriptor = NULL; 

	if (!CreatePipe(&hInput[0], &hInput[1], &saAttr, PIPE_BUFSIZ))
		ax_err_goto(out);
	if (!SetHandleInformation(hInput[0], HANDLE_FLAG_INHERIT, TRUE)
			|| !SetHandleInformation(hInput[1], HANDLE_FLAG_INHERIT, FALSE))
		ax_err_goto(out);

	if (!CreatePipe(&hOutput[0], &hOutput[1], &saAttr, PIPE_BUFSIZ))
		ax_err_goto(out);
	if (!SetHandleInformation(hOutput[1], HANDLE_FLAG_INHERIT, TRUE)
			|| !SetHandleInformation(hOutput[0], HANDLE_FLAG_INHERIT, FALSE))
		ax_err_goto(out);

	if (!CreatePipe(&hError[0], &hError[1], &saAttr, PIPE_BUFSIZ))
		ax_err_goto(out);
	if (!SetHandleInformation(hError[1], HANDLE_FLAG_INHERIT, TRUE)
			|| !SetHandleInformation(hError[0], HANDLE_FLAG_INHERIT, FALSE))
		ax_err_goto(out);

	DWORD dwProcessId;
	hProcess = CreateChildProcess(pwzFile, pwzCmdLine, hInput[0],
			hOutput[1], hError[1], &dwProcessId);
	if (hProcess == INVALID_HANDLE_VALUE)
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

ax_proc *ax_proc_open(const ax_uchar* file, ax_uchar *const argv[])
{
	ax_proc *pProc = NULL;
	LPWSTR pCmdBuf = NULL;
	CONST DWORD dwBufLen = 32 * 1024;

	if (!(pCmdBuf = HeapAlloc(GetProcessHeap(), 0, dwBufLen * sizeof(ax_uchar))))
		ax_err_goto(fail);
	DWORD dwBufOffset = 0;

	for (int i = 0; argv[i]; i++) {
		/* Construct sequence with <space,",arg[i],"> */
		BOOL bQuote = FALSE, bEscape = FALSE;
		int j;
		for (j = 0; argv[i][j]; j++) {
			if (argv[i][j] == L' ' || argv[i][j] == L'\t') {
				bQuote = TRUE;
			}
			else if (argv[i][j] == L'"' || argv[i][j] == L'\\') {
				bEscape = TRUE;
			}
		}
		if (j == 0)
			bQuote = TRUE; /* argv[i] is empty, quote it */

		LPWSTR pwzEscaped = argv[i];
		if (bEscape) {
			/* Escape special token " and \ to \" and \\*/
			LPWSTR pwzEscapeSlash = NULL;
			if (!(pwzEscapeSlash = ax_wcsrepl(argv[i], L"\\", L"\\\\")))
				goto fail;

			if (!(pwzEscaped = ax_wcsrepl(pwzEscapeSlash, L"\"", L"\\\""))) {
				free(pwzEscapeSlash);
				goto fail;
			}
			free(pwzEscapeSlash);
		}

		/* Do not append space char at first time, notice !!i */
		DWORD dwAppendLen = lstrlenW(pwzEscaped) + !!bQuote * 2 + !!i;

		if (dwAppendLen + dwBufOffset >= dwBufLen) {
			if (bEscape)
				free(pwzEscaped);
			errno = AX_ENOBUFS;
			goto fail;
		}

		lstrcpyW(pCmdBuf + dwBufOffset, ((LPCWSTR)L" " + !i));
		if (bQuote)
			lstrcatW(pCmdBuf + dwBufOffset, L"\"");
		lstrcatW(pCmdBuf + dwBufOffset, pwzEscaped);
		if (bQuote)
			lstrcatW(pCmdBuf + dwBufOffset, L"\"");
		dwBufOffset += dwAppendLen;

		if (bEscape)
			free(pwzEscaped);
	}

	if (!(pProc = malloc(sizeof(*pProc))))
		goto fail;

	if (!ExecuteWithPipe(file, pCmdBuf, pProc))
		goto fail;

	HeapFree(GetProcessHeap(), 0, pCmdBuf);
	return pProc;
fail:
	HeapFree(GetProcessHeap(), 0, pCmdBuf);
	free(pProc);
	return NULL;
}

int ax_proc_kill(const ax_proc *proc)
{
	if (!proc) {
		errno = EINVAL;
		return -1;
	}
	if (!TerminateProcess(proc->hProcess, 1))
		ax_err_goto(fail);
	return 0;
fail:
	return -1;
}

int ax_proc_close(ax_proc *proc)
{
	int exit_code = -1;
	DWORD dwExitCode = 0;

	if (!proc) {
		errno = AX_EINVAL;
		goto out;
	}

	if (proc->in)
		fclose(proc->in);
	if (proc->out)
		fclose(proc->out);
	if (proc->err)
		fclose(proc->err);

	if (WaitForSingleObject(proc->hProcess, INFINITE))
		ax_err_goto(out);

	if (!GetExitCodeProcess(proc->hProcess, &dwExitCode)) {
		ax_err_goto(out);
	}
	exit_code = dwExitCode;
out:
	CloseHandle(proc->hProcess);
	free(proc);
	return exit_code;
}

#else

#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>

#define CLOSE_PIPE(pipe) do { close((pipe)[0]); close((pipe)[1]); } while(0)

/* To close all fd inherited after forked */
static ax_proc *sg_proc_chain;
static pthread_mutex_t sg_lock = PTHREAD_MUTEX_INITIALIZER;

struct ax_proc_st
{
	struct ax_proc_st *next;
	pid_t pid;
	FILE *in, *out, *err;
};

static int _do_popen3(ax_proc *proc, const ax_uchar *file, ax_uchar *const argv[])
{
	int retval = -1;
	int child_in[2] = { -1 };
	int child_out[2] = { -1 };
	int child_err[2] = { -1 };

	if (pipe(child_in) || pipe(child_out) || pipe(child_err)) {
		ax_error_occur();
		goto out;
	}

	pid_t cpid = fork();
	if (cpid < 0) {
		ax_error_occur();
		goto out;
	}
	if (cpid == 0) {
		if (dup2(child_in[0], 0) == -1)
			_Exit(127);
		if (dup2(child_out[1], 1) == -1)
			_Exit(127);
		if (dup2(child_err[1], 2) == -1)
			_Exit(127);

		CLOSE_PIPE(child_in);
		CLOSE_PIPE(child_out);
		CLOSE_PIPE(child_err);

		for (ax_proc *p = sg_proc_chain; p; p = p->next) {
			close(fileno(p->in));
			close(fileno(p->out));
			close(fileno(p->err));
		}
		execv(file, argv);
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

ax_proc *ax_proc_open(const ax_uchar* file, ax_uchar *const argv[])
{
	ax_proc *proc = malloc(sizeof(*proc));
	if (!proc)
		return NULL;

	if (0 > _do_popen3(proc, file, argv)) {
		free(proc);
		return NULL;
	}

	pthread_mutex_lock(&sg_lock);
	proc->next = sg_proc_chain;
	sg_proc_chain = proc;
	pthread_mutex_unlock(&sg_lock);

	return proc;
}

int ax_proc_kill(const ax_proc *proc)
{
	int retval;
	if ((retval = kill(proc->pid, SIGKILL)))
		ax_error_occur();
	return retval;
}

int ax_proc_close(ax_proc *proc)
{
	pid_t wait_pid;
	pid_t pid = proc->pid;

	ax_proc **p = &sg_proc_chain;
	while (*p) {
		if (*p == proc)
			goto found;
		p = &(*p)->next;
	}
	errno = EINVAL;
	return -1;

found:
	*p = (*p)->next;

	if (proc->in)
		fclose(proc->in);
	if (proc->out)
		fclose(proc->out);
	if (proc->err)
		fclose(proc->err);
	free(proc);

	int status = -1;
	while (1) {
		wait_pid = waitpid(pid, &status, 0);
		if (wait_pid == -1) {
			if (errno == EINTR)
				continue;
			ax_error_occur();
			return -1;
		}

		if (WIFEXITED(status))
			return WEXITSTATUS(status);
		else if (WIFSIGNALED(status))
			return 1;
	}
}
#endif

FILE *ax_proc_stdio(const ax_proc *proc, int fd)
{
	switch (fd) {
		case 0: return proc->in;
		case 1: return proc->out;
		case 2: return proc->err;
		default:
			errno = AX_EINVAL;
			return NULL;
	}
}

int ax_proc_fclose(ax_proc *proc, int fd)
{
	int retval = -1;
	if (!proc) {
		errno = EINVAL;
		return -1;
	}

	switch (fd) {
		case 0:
			retval = fclose(proc->in);
			proc->in = NULL;
			break;
		case 1:
			retval = fclose(proc->out);
			proc->out = NULL;
			break;
		case 2:
			retval = fclose(proc->err);
			proc->err = NULL;
			break;
		default:
			errno = AX_EINVAL;
			break;
	}
	return retval;
}

pid_t ax_proc_pid(const ax_proc *proc)
{
	return proc->pid;
}

