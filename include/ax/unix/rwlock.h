#ifndef AX_WIN32_RWLOCK_H
#define AX_WIN32_RWLOCK_H

#include <pthread.h>
#include <assert.h>

struct ax_rwlock_st
{
	pthread_rwlock_t rwlock;
};

#ifndef AX_RWLOCK_DEFINED
#define AX_RWLOCK_DEFINED
typedef struct ax_rwlock_st ax_rwlock;
#endif

static inline int ax_rwlock_init(ax_rwlock *lock)
{
	assert(lock);
	return - !!pthread_rwlock_init(&lock->rwlock, NULL);
}

static inline int ax_rwlock_rlock(ax_rwlock *lock)
{
	assert(lock);
	return - !!pthread_rwlock_rdlock(&lock->rwlock);
}

static inline int ax_rwlock_try_rlock(ax_rwlock *lock)
{
	assert(lock);
	return - !!pthread_rwlock_tryrdlock(&lock->rwlock);
}

static inline int ax_rwlock_wlock(ax_rwlock *lock)
{
	assert(lock);
	return - !!pthread_rwlock_wrlock(&lock->rwlock);
}

static inline int ax_rwlock_try_wlock(ax_rwlock *lock)
{
	assert(lock);
	return - !!pthread_rwlock_trywrlock(&lock->rwlock);
}

static inline int ax_rwlock_unlock(ax_rwlock *lock)
{
	assert(lock);
	return - !!pthread_rwlock_unlock(&lock->rwlock);
}

static inline void ax_rwlock_destroy(ax_rwlock *lock)
{
	assert(lock);
	(void)pthread_rwlock_destroy(&lock->rwlock);
}

#endif
