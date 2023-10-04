#ifndef AX_MUTEX_H
#define AX_MUTEX_H

#include "detect.h"
#include <errno.h>
#include <assert.h>

#ifdef AX_OS_WIN
#include <synchapi.h>
#include <windows.h>
#define AX_MUTEX_INIT { .section = NULL }
#else
#include <pthread.h>
#define AX_MUTEX_INIT { .mutex = PTHREAD_MUTEX_INITIALIZER }
#endif

#ifndef AX_THREAD_DEFINED
#define AX_THREAD_DEFINED
typedef struct ax_thread_st ax_thread;
#endif

#ifndef AX_MUTEX_DEFINED
#define AX_MUTEX_DEFINED
typedef struct ax_mutex_st ax_mutex;
#endif


struct ax_mutex_st
{
#ifdef AX_OS_WIN
	CRITICAL_SECTION *section;
#else
	pthread_mutex_t mutex;
#endif
};

static inline int ax_mutex_init(ax_mutex *lock)
{
#ifdef AX_OS_WIN
	if (!(lock->section = HeapAlloc(GetProcessHeap(), 0, sizeof *lock->section)))
		return -1;
	InitializeCriticalSection(lock->section);
#else
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	if (pthread_mutex_init(&lock->mutex, &attr))
		return -1;
#endif
	return 0;
}


#ifdef AX_OS_WIN
static inline int __ax_mutex_init_win32(ax_mutex *lock)
{
	if (!lock->section)
	{
		CRITICAL_SECTION *secp = HeapAlloc(GetProcessHeap(), 0, sizeof *secp);
		if (!secp) {
			return -1;
		}
		InitializeCriticalSection(secp);
		if (InterlockedCompareExchangePointer((PVOID*)&lock->section, secp, NULL)) {
			DeleteCriticalSection(lock->section);
			HeapFree(GetProcessHeap(), 0, secp);
		}
	}
}
#endif

static inline int ax_mutex_lock(ax_mutex *lock)
{

#ifdef AX_OS_WIN
	if (__ax_mutex_init_win32(lock))
		return -1;
	EnterCriticalSection(lock->section);
#else
	if (pthread_mutex_lock(&lock->mutex))
		return -1;
#endif
	return 0;
}

static inline int ax_mutex_trylock( ax_mutex *lock)
{
#ifdef AX_OS_WIN
	if (__ax_mutex_init_win32(lock))
		return -1;
	if (TryEnterCriticalSection(lock->section) == 0) {
		return 1;
	}
#else
	int ret = pthread_mutex_trylock(&lock->mutex);
	if (ret == EBUSY)
		return 1;
	if (ret) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_mutex_unlock( ax_mutex *lock)
{
#ifdef AX_OS_WIN
	if (!lock->section) {
		errno = EPERM;
		return -1;
	}
	LeaveCriticalSection(lock->section);
#else
	if (pthread_mutex_unlock(&lock->mutex))
		return -1;
#endif
	return 0;
}

static inline void ax_mutex_destroy( ax_mutex *lock)
{
	assert(lock != NULL);
#ifdef AX_OS_WIN
	if (!lock->section)
		return;
	DeleteCriticalSection(lock->section);
	HeapFree(GetProcessHeap(), 0, lock->section);
#else
	(void)pthread_mutex_destroy(&lock->mutex);
#endif
}

#endif
