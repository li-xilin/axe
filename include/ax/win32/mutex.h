#ifndef AX_WIN32_MUTEX_H
#define AX_WIN32_MUTEX_H

#include <windows.h>
#include <process.h>
#include <errno.h>
#include <assert.h>

#ifndef AX_THREAD_DEFINED
#define AX_THREAD_DEFINED
typedef HANDLE ax_thread;
#endif

#ifndef AX_MUTEX_DEFINED
#define AX_MUTEX_DEFINED
typedef struct ax_mutex_st ax_mutex;
#endif

struct ax_mutex_st
{
	CRITICAL_SECTION section;
};

static inline int ax_mutex_init(ax_mutex *lock)
{
	InitializeCriticalSection(&lock->section);
	return 0;
}

static inline int ax_mutex_lock(ax_mutex *lock)
{
	EnterCriticalSection(&lock->section);
	return 0;
}

static inline int ax_mutex_trylock( ax_mutex *lock)
{
	if (TryEnterCriticalSection(&lock->section))
		return 0;
	return 1;
}

static inline int ax_mutex_unlock( ax_mutex *lock)
{
	LeaveCriticalSection(&lock->section);
	return 0;
}

static inline int ax_mutex_destroy( ax_mutex *lock)
{
	DeleteCriticalSection(&lock->section);
	return 0;
}

#endif
