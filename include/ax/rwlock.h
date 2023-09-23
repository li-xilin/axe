/*
 * Copyright (c) 2023 Li xilin <lixilin@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef AX_RWLOCK_H
#define AX_RWLOCK_H

#include <assert.h>
#include <errno.h>

#ifdef AX_OS_WIN
#include <synchapi.h>
#define AX_RWLOCK_INIT { .bExclusive = FALSE, .SrwLock = SRWLOCK_INIT }
#else
#include <pthread.h>
#define AX_RWLOCK_INIT { .rwlock = PTHREAD_RWLOCK_INITIALIZER }
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
