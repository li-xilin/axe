#ifndef AX_WIN32_RWLOCK_H
#define AX_WIN32_RWLOCK_H

#include <assert.h>
#include <errno.h>

#ifdef AX_OS_WIN
#include <synchapi.h>
#else
#include <pthread.h>
#endif

struct ax_rwlock_st
{
#ifdef AX_OS_WIN
	SRWLOCK SrwLock;
	BOOLEAN bExclusive;
#else
	pthread_rwlock_t rwlock;
#endif
};

#ifndef AX_RWLOCK_DEFINED
#define AX_RWLOCK_DEFINED
typedef struct ax_rwlock_st ax_rwlock;
#endif

static inline int ax_rwlock_init(ax_rwlock *lock)
{
	assert(lock);
#ifdef AX_OS_WIN
	InitializeSRWLock(&lock->SrwLock);
	lock->bExclusive = FALSE;
#else

	if (pthread_rwlock_init(&lock->rwlock, NULL)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_rwlock_rlock(ax_rwlock *lock)
{
	assert(lock);
#ifdef AX_OS_WIN
	AcquireSRWLockShared(&lock->SrwLock);
#else
	if (pthread_rwlock_rdlock(&lock->rwlock)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_rwlock_try_rlock(ax_rwlock *lock)
{
	assert(lock);
#ifdef AX_OS_WIN
	if (!TryAcquireSRWLockShared(&lock->SrwLock)) {
		return -1;
	}
#else
	if (pthread_rwlock_tryrdlock(&lock->rwlock)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_rwlock_wlock(ax_rwlock *lock)
{
	assert(lock);
#ifdef AX_OS_WIN
	AcquireSRWLockExclusive(&lock->SrwLock);
	lock->bExclusive = TRUE;
#else

	if (pthread_rwlock_wrlock(&lock->rwlock)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_rwlock_try_wlock(ax_rwlock *lock)
{
	assert(lock);
#ifdef AX_OS_WIN
	if (!TryAcquireSRWLockExclusive(&lock->SrwLock))
		return 1;
	lock->bExclusive = TRUE;
#else
	int ret = pthread_rwlock_trywrlock(&lock->rwlock);
	if (ret == EBUSY)
		return 1;
	if (ret) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_rwlock_unlock(ax_rwlock *lock)
{
	assert(lock);
#ifdef AX_OS_WIN
	if (lock->bExclusive) {
		lock->bExclusive = FALSE;
		ReleaseSRWLockExclusive(&lock->SrwLock);
	} else
		ReleaseSRWLockShared(&lock->SrwLock);
#else
	if (pthread_rwlock_unlock(&lock->rwlock)) {
		return -1;
	}
#endif
	return 0;
}

static inline void ax_rwlock_destroy(ax_rwlock *lock)
{
	assert(lock);
#ifndef AX_OS_WIN
	(void)pthread_rwlock_destroy(&lock->rwlock);
#endif
}

#endif
