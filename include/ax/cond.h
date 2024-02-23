#ifndef AX_COND_H
#define AX_COND_H

#include "mutex.h"
#include "errno.h"
#include <stdint.h>
#include <assert.h>
#include <time.h>
#include <errno.h>

#ifdef AX_OS_WIN
#include <synchapi.h>
#define AX_COND_INIT { .condvar = NULL }
#else
#include <pthread.h>
#define AX_COND_INIT { .cond = PTHREAD_COND_INITIALIZER }
#endif

struct ax_cond_st
{
#ifdef AX_OS_WIN
	CONDITION_VARIABLE *condvar;
#else
	pthread_cond_t cond;
#endif
};

#ifndef AX_COND_DEFINED
#define AX_COND_DEFINED
typedef struct ax_cond_st ax_cond;
#endif

#ifdef AX_OS_WIN
static inline int __ax_cond_init_competitive(ax_cond *cond)
{
	if (!cond->condvar) {
		CONDITION_VARIABLE *condp = HeapAlloc(GetProcessHeap(), 0, sizeof *condp);
		if (!condp) {
			return -1;
		}
		if (InterlockedCompareExchangePointer(&cond->condvar, condp, NULL)) {
			HeapFree(GetProcessHeap(), 0, condp);
		}
	}
	return 0;
}
#endif

static inline int ax_cond_init(ax_cond *cond)
{
	assert(cond);
#ifdef AX_OS_WIN
	if (!(cond->condvar = HeapAlloc(GetProcessHeap(), 0, sizeof *cond->condvar))) {
		return -1;
	}
	InitializeConditionVariable(cond->condvar);
#else
	if (pthread_cond_init(&cond->cond, NULL)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_cond_sleep(ax_cond *cond, ax_mutex *mutex, int millise)
{
	assert(cond);
	assert(mutex);
#ifdef AX_OS_WIN
	if (!mutex->section) {
		errno = EPERM;
		return -1;
	}
	if (__ax_cond_init_competitive(cond)) {
		ax_error_occur();
		return -1;
	}
	
	if (!SleepConditionVariableCS(cond->condvar, mutex->section,
				millise < 0 ? INFINITE : millise)) {
		ax_error_occur();
		if (GetLastError() == ERROR_TIMEOUT)
			return 1;
		return -1;
	}
#else
	if (millise < 0)
		return - !!pthread_cond_wait(&cond->cond, &mutex->mutex);

        const int NS_PER_MS = (1000 * 1000 * 1000);
        struct timespec spec;
        if (clock_gettime(CLOCK_REALTIME, &spec) == -1)
                return -1;
        uint32_t sec = millise / 1000;
        uint32_t nsec = (millise % 1000) * 1000 * 1000;
        if ((NS_PER_MS - spec.tv_nsec) < nsec)
                spec.tv_sec += sec, spec.tv_nsec += nsec;
        else
                spec.tv_sec += sec + 1, spec.tv_nsec += (spec.tv_nsec + nsec) % NS_PER_MS;

	// Shall not return EINTR
        int ret = pthread_cond_timedwait(&cond->cond, &mutex->mutex, &spec);
        if (ret == ETIMEDOUT)
                return 1;
        if (!ret)
                return 0;
        else
                return -1;

#endif
	return 0;
}

static inline int ax_cond_wake(ax_cond *cond)
{
	assert(cond);
#ifdef AX_OS_WIN
	if (__ax_cond_init_competitive(cond)) {
		ax_error_occur();
		return -1;
	}
	WakeConditionVariable(cond->condvar);
#else
	if (pthread_cond_signal(&cond->cond)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_cond_wake_all(ax_cond *cond)
{
	assert(cond);
#ifdef AX_OS_WIN
	if (__ax_cond_init_competitive(cond)) {
		ax_error_occur();
		return -1;
	}
	WakeAllConditionVariable(cond->condvar);
#else
	if (pthread_cond_broadcast(&cond->cond)) {
		return -1;
	}
#endif
	return 0;
}

static inline void ax_cond_destroy(ax_cond *cond)
{
	assert(cond);
#ifdef AX_OS_WIN
	if (!cond->condvar)
		return;
	HeapFree(GetProcessHeap(), 0, cond->condvar);
#else
	(void)pthread_cond_destroy(&cond->cond);
#endif
}

#endif

