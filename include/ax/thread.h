#ifndef AX_THREAD_H
#define AX_THREAD_H

#include "debug.h"
#include "detect.h"
#include <errno.h>
#include <assert.h>
#include <stdlib.h>
#include <stdint.h>

#ifdef AX_OS_WIN
#include <windows.h>
#include <process.h>
struct ax_thread_st
{
	HANDLE hThread;
};
#else
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
struct ax_thread_st
{
	pthread_t thread;
};
#endif

typedef uintptr_t (ax_thread_func_f)(void *arg);

#ifndef AX_THREAD_DEFINED
#define AX_THREAD_DEFINED
typedef struct ax_thread_st ax_thread;
#endif

static inline ax_thread ax_thread_self(void)
{
#ifdef AX_OS_WIN
	return (ax_thread) { .hThread = GetCurrentThread() };
#else
        return (ax_thread) { .thread = pthread_self() };
#endif
}

struct __ax_thread_argument_st
{
        void *arg;
        ax_thread_func_f *func;
};

#ifdef AX_OS_WIN
static DWORD WINAPI __ax_thread_proc(void *arg)
#else
static void * __ax_thread_proc(void *arg)
#endif
{
        struct __ax_thread_argument_st ta =
		*(struct __ax_thread_argument_st *)arg;
	free(arg);
#ifdef AX_OS_WIN
	return (DWORD)ta.func(ta.arg);
#else
	return (void *)ta.func(ta.arg);
#endif
}

static inline int ax_thread_create(ax_thread_func_f *thread_func, void *arg, ax_thread *thread)
{
	assert(thread_func);
	assert(thread);

	struct __ax_thread_argument_st *ta = NULL;
	
	if (!(ta = malloc(sizeof *ta)))
		return -1;
	ta->arg = arg;
	ta->func = thread_func;

#ifdef AX_OS_WIN
	DWORD dwThreadId;

	thread->hThread = CreateThread(NULL, 0, __ax_thread_proc, ta, 0, &dwThreadId);
	if (!thread->hThread) {
		free(ta);
		return -1;
	}
	return 0;
#else
	if (pthread_create(&thread->thread, NULL, __ax_thread_proc, ta)) {
		free(ta);
		return -1;
	}
	return 0;
#endif
}

static inline void ax_thread_exit(uintptr_t ret_code)
{
#ifdef AX_OS_WIN
	ExitThread(ret_code);
#else
	pthread_exit((void *)ret_code);
#endif
}

static inline void ax_thread_sleep(unsigned int millise)
{
#ifdef AX_OS_WIN
	ax_assert(millise < UINT_MAX / 1000, "millise is too big");
	Sleep(millise);
#else
	int sec = millise / 1000;
	int msec = millise % 1000;
	sleep(sec);
	usleep(msec * 1000);
#endif
}

static inline int ax_thread_join(ax_thread *thread, uintptr_t *retval)
{
	assert(thread);
	assert(retval);
#ifdef AX_OS_WIN
	DWORD rval = -1;

	if(WaitForSingleObject(thread->hThread, INFINITE))
		return -1;

	if (!GetExitCodeThread(thread->hThread, &rval))
		return -1;

	*retval = rval;
	(void)CloseHandle(thread->hThread);
	return 0;
#else
	void *retptr;
	int err = pthread_join(thread->thread, &retptr);
	if(err) {
		return -1;
	}
	if (retptr == PTHREAD_CANCELED)
		return -1;
	*retval = (uintptr_t)retptr;
	return 0;

#endif
}

static inline int ax_thread_detach(ax_thread *thread)
{
	assert(thread);
#ifdef AX_OS_WIN
	return CloseHandle(thread->hThread);
#else
	return - !!pthread_detach(thread->thread);
#endif
}

static inline void ax_thread_yield(void)
{
#ifdef AX_OS_WIN
	SwitchToThread();
#else
	extern int sched_yield(void);
	sched_yield();
#endif
}

static inline void ax_thread_kill(ax_thread *thread)
{
#ifdef AX_OS_WIN
	(void)TerminateThread(thread->hThread, 0);
#else
	(void)pthread_cancel(thread->thread);
#endif
}

#endif

