#ifndef AX_WIN32_RWLOCK_H
#define AX_WIN32_RWLOCK_H

#include <ax/def.h>
#include <synchapi.h>
#include <assert.h>

struct ax_rwlock_st
{
	SRWLOCK SrwLock;
	BOOLEAN bExclusive;
};

#ifndef AX_RWLOCK_DEFINED
#define AX_RWLOCK_DEFINED
typedef struct ax_rwlock_st ax_rwlock;
#endif

static inline int ax_rwlock_init(ax_rwlock *lock)
{
	assert(lock);
	InitializeSRWLock(&lock->SrwLock);
	lock->bExclusive = FALSE;
	return 0;
}

static inline int ax_rwlock_rlock(ax_rwlock *lock)
{
	assert(lock);
	AcquireSRWLockShared(&lock->SrwLock);
	return 0;
}

static inline int ax_rwlock_try_rlock(ax_rwlock *lock)
{
	assert(lock);
	return !TryAcquireSRWLockShared(&lock->SrwLock);
}

static inline int ax_rwlock_wlock(ax_rwlock *lock)
{
	assert(lock);
	AcquireSRWLockExclusive(&lock->SrwLock);
	lock->bExclusive = TRUE;
	return 0;
}

static inline int ax_rwlock_try_wlock(ax_rwlock *lock)
{
	assert(lock);
	if (!TryAcquireSRWLockExclusive(&lock->SrwLock))
		return 1;
	lock->bExclusive = TRUE;
	return 0;
}

static inline int ax_rwlock_unlock(ax_rwlock *lock)
{
	assert(lock);
	if (lock->bExclusive) {
		lock->bExclusive = FALSE;
		ReleaseSRWLockExclusive(&lock->SrwLock);
	} else
		ReleaseSRWLockShared(&lock->SrwLock);
	return 0;
}

static inline void ax_rwlock_destroy(ax_rwlock *lock)
{
	assert(lock);
	ax_unused(lock);
}

#endif
