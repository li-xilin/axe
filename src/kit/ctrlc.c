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

#include "ax/ctrlc.h"
#include "ax/detect.h"
#include "ax/errno.h"

static ax_ctrlc_proc_f *sg_user_handler = NULL;

#ifdef AX_OS_WIN
#include <windows.h>
BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
	switch( fdwCtrlType)
	{
		case CTRL_C_EVENT:
			sg_user_handler(AX_CTRLC_INT);
			return TRUE;
		case CTRL_BREAK_EVENT:
			sg_user_handler(AX_CTRLC_QUIT);
			return TRUE;
		case CTRL_CLOSE_EVENT:
		case CTRL_LOGOFF_EVENT:
		case CTRL_SHUTDOWN_EVENT:
			sg_user_handler(AX_CTRLC_CLOSE);
			return TRUE;
		default:
			return FALSE;
	}
}

int ax_ctrlc_set(ax_ctrlc_proc_f *proc)
{
	if (!proc) {
		errno = AX_EINVAL;
		return -1;
	}

	if (sg_user_handler == NULL) {
		if(!SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler,TRUE)) {
			ax_error_occur();
			return -1;
		}
	}
	sg_user_handler = proc;
	return 0;
}

void ax_ctrlc_unset(void)
{
	if (!sg_user_handler)
		return;
	SetConsoleCtrlHandler((PHANDLER_ROUTINE) CtrlHandler,FALSE);
	sg_user_handler = NULL;
}

#else

#include <stdlib.h>
#include <signal.h>
#include <pthread.h>
#include <unistd.h>

struct sigaction sg_sigint_sa;
struct sigaction sg_sigterm_sa;
struct sigaction sg_sigquit_sa;

void *thread_proc(void *arg)
{
	int action = *(int *)arg;
	free(arg);
	sg_user_handler(action);
	return NULL;
}

static void signal_handler (int sig)
{
	int *act = malloc(sizeof(int));
	if (!act)
		return;
	switch (sig) {
		case SIGINT:
			*act = AX_CTRLC_INT;
			break;
		case SIGQUIT:
			*act = AX_CTRLC_QUIT;
			break;
		case SIGTERM:
			*act = AX_CTRLC_CLOSE;
			break;
	}
	pthread_t thd;
	if (pthread_create(&thd, NULL, thread_proc, act))
		return;
	if (sig == SIGTERM) {
		pthread_join(thd, 0);
		_exit(143);
	}
	pthread_detach(thd);
}

int ax_ctrlc_set(ax_ctrlc_proc_f *proc)
{
	if (!proc) {
		errno = AX_EINVAL;
		return -1;
	}

	if (sg_user_handler == NULL) {
		struct sigaction sa = {
			.sa_handler = signal_handler,
			.sa_flags = SA_NODEFER | SA_RESTART,
		};
		if (sigaction(SIGINT, &sa, &sg_sigint_sa))
			ax_error_goto(fail);
		if (sigaction(SIGQUIT, &sa, &sg_sigquit_sa))
			ax_error_goto(fail);
		if (sigaction(SIGTERM, &sa, &sg_sigterm_sa))
			ax_error_goto(fail);
	}
	sg_user_handler = proc;
	return 0;
fail:
	return -1;
}

void ax_ctrlc_unset(void)
{
	if (!sg_user_handler)
		return;
	(void)sigaction(SIGINT, &sg_sigint_sa, NULL);
	(void)sigaction(SIGQUIT, &sg_sigquit_sa, NULL);
	(void)sigaction(SIGTERM, &sg_sigterm_sa, NULL);
	sg_user_handler = NULL;
}


#endif


