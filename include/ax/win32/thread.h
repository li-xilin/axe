#ifndef AX_WIN32_THREAD_H
#define AX_WIN32_THREAD_H

#include "ax/debug.h"
#include <windows.h>
#include <process.h>
#include <errno.h>
#include <assert.h>

#ifndef AX_THREAD_FUNC_F_DEFINED
#define AX_THREAD_FUNC_F_DEFINED
typedef uintptr_t (ax_thread_func_f)(void *arg);
#endif

struct ax_thread_st
{
	HANDLE hThread;
};

#ifndef AX_THREAD_DEFINED
#define AX_THREAD_DEFINED
typedef struct ax_thread_st ax_thread;
#endif

static inline ax_thread ax_thread_self(void)
{
	return (ax_thread) { .hThread = GetCurrentThread() };
}

struct ThreadArgument
{
	LPVOID lpArg;
	ax_thread_func_f *fnThreadFunc;
};

static DWORD WINAPI ThreadFunc(LPVOID lpParam)
{
	struct ThreadArgument ta = *(struct ThreadArgument *)lpParam;
	free(lpParam);
	return ta.fnThreadFunc(ta.lpArg);
}

static inline int ax_thread_create(ax_thread_func_f *thread_func, void *arg, ax_thread *thread)
{
	assert(thread_func);
	assert(thread);

	DWORD dwThreadId;
	struct ThreadArgument *ta = malloc(sizeof *ta);
	if (!ta)
		return -1;

	*ta = (struct ThreadArgument) {
		.lpArg = arg,
		.fnThreadFunc = thread_func
	};
	HANDLE hThread = CreateThread(NULL, 0, ThreadFunc, ta, 0, &dwThreadId);
	if (!hThread) {
		free(ta);
		return -1;
	}
	thread->hThread = hThread;
	return 0;
}

static inline void ax_thread_exit(uintptr_t rval)
{
	ExitThread(rval);
}

static inline void ax_thread_sleep(unsigned int millise)
{
	ax_assert(millise < UINT_MAX / 1000, "millise is too big");
	Sleep(millise);
}

static inline int ax_thread_join(ax_thread *thread, uintptr_t *retval)
{
	assert(thread);
	assert(retval);
	DWORD rval = -1;

	if(WaitForSingleObject(thread->hThread, INFINITE))
		return -1;

	if (!GetExitCodeThread(thread->hThread, &rval))
		return -1;

	*retval = rval;
	(void)CloseHandle(thread->hThread);
	return 0;
}

// http://stackoverflow.com/questions/12744324/how-to-detach-a-thread-on-windows-c
static inline int ax_thread_detach(ax_thread *thread)
{
	assert(thread);
	return CloseHandle(thread->hThread);
}

static inline void ax_thread_yield(void)
{
	SwitchToThread();
}

static inline void ax_thread_kill(ax_thread *thread)
{
	(void)TerminateThread(thread->hThread, 0);
}

#endif
