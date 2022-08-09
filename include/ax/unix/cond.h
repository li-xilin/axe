#ifndef AX_UNIX_COND_H
#define AX_UNIX_COND_H

#include "mutex.h"
#include <pthread.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>

struct ax_cond_st
{
	pthread_cond_t cond;
};

#define AX_COND_INIT { .cond = PTHREAD_COND_INITIALIZER }

#ifndef AX_COND_DEFINED
#define AX_COND_DEFINED
typedef struct ax_cond_st ax_cond;
#endif

static inline int ax_cond_init(ax_cond *cond)
{
	assert(cond);
	return !!pthread_cond_init(&cond->cond, NULL);
}

static inline int ax_cond_sleep(ax_cond *cond, ax_mutex *mutex, int millise)
{
	assert(cond);
	assert(mutex);

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

	// pthread_cond_timedwait shall not return an error code of [EINTR].
        int ret = pthread_cond_timedwait(&cond->cond, &mutex->mutex, &spec);
        if (ret == ETIMEDOUT)
                return 1;
        if (!ret)
                return 0;
        else
                return -1;	
}

static inline int ax_cond_wake(ax_cond *cond)
{
	assert(cond);
	return - !!pthread_cond_signal(&cond->cond);
}

static inline int ax_cond_wake_all(ax_cond *cond)
{
	assert(cond);
	return - !!pthread_cond_broadcast(&cond->cond);
}

static inline void ax_cond_destroy(ax_cond *cond)
{
	assert(cond);
	(void)pthread_cond_destroy(&cond->cond);
}

#endif
