#ifndef AX_UNIX_MUTEX_H
#define AX_UNIX_MUTEX_H

#include <assert.h>
#include <pthread.h>
#include <errno.h>

#ifndef AX_MUTEX_DEFINED
#define AX_MUTEX_DEFINED
typedef struct ax_mutex_st ax_mutex;
#endif

#define AX_MUTEX_INIT { .mutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP }

struct ax_mutex_st
{
	pthread_mutex_t mutex;
};

static inline int ax_mutex_init(ax_mutex *lock)
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	return - !!pthread_mutex_init(&lock->mutex, &attr);
}

static inline int ax_mutex_lock(ax_mutex *lock)
{
	return - !!pthread_mutex_lock(&lock->mutex);
}

static inline int ax_mutex_trylock( ax_mutex *lock)
{
	int ret = pthread_mutex_trylock(&lock->mutex);
	if (ret == EBUSY)
		return 1;
	if (!ret)
		return 0;
	return -1;
}

static inline int ax_mutex_unlock( ax_mutex *lock)
{
	return - !!pthread_mutex_unlock(&lock->mutex);
}

static inline void ax_mutex_destroy( ax_mutex *lock)
{
	(void)pthread_mutex_destroy(&lock->mutex);
}

#endif
