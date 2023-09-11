#ifndef AX_SEM_H
#define AX_SEM_H

#include <assert.h>
#include <errno.h>


#ifdef AX_OS_WIN
#include <synchapi.h>
#else
#include <semaphore.h>
#include <time.h>
#endif

struct ax_sem_st
{
#ifdef AX_OS_WIN
	HANDLE hSem;
#else
	sem_t sem;
#endif
};

#ifndef AX_SEM_DEFINED
#define AX_SEM_DEFINED
typedef struct ax_sem_st ax_sem;
#endif

static inline int ax_sem_init(ax_sem *sem, size_t initial_value)
{
	assert(sem);
#ifdef AX_OS_WIN
	SECURITY_ATTRIBUTES Attr = {
		.nLength = sizeof Attr,
		.lpSecurityDescriptor = NULL,
		.bInheritHandle = TRUE,
	};
	if (!CreateSemaphoreA(&Attr, initial_value, ULONG_MAX, NULL)) {
		return -1;
	}
#else
	if (sem_init(&sem->sem, 0, initial_value)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_sem_post(ax_sem *sem)
{
	assert(sem);
#ifdef AX_OS_WIN
	if (!ReleaseSemaphore(sem->hSem, 1, NULL)) {
		return -1;
	}
#else
	if (sem_post(&sem->sem)) {
		return -1;
	}
#endif
	return 0;
}

static inline int ax_sem_wait(ax_sem *sem, int millise)
{
	assert(sem);
#ifdef AX_OS_WIN
	DWORD ret = WaitForSingleObject(sem->hSem, millise < 0 ? INFINITE : millise);
	if (ret == WAIT_FAILED)
		return -1;
	if (ret == WAIT_TIMEOUT)
		return 1;
	return 0;
#else
	if (millise < 0)
		return sem_wait(&sem->sem);

	const int NS_PER_MS = (1000 * 1000 * 1000);

	struct timespec spec;
	if (clock_gettime(CLOCK_REALTIME, &spec) == -1)
		return -1;
	int sec = millise / 1000;
	int nsec = (millise % 1000) * 1000 * 1000;
	if ((NS_PER_MS - spec.tv_nsec) < nsec)
		spec.tv_sec += sec, spec.tv_nsec += nsec;
	else
		spec.tv_sec += sec + 1, spec.tv_nsec += (spec.tv_nsec + nsec) % NS_PER_MS;

	int ret;
	while ((ret = sem_timedwait(&sem->sem, &spec)) == -1 && errno == EINTR)
		continue;
	if (!ret)
		return 0;
	if (errno == ETIMEDOUT)
		return 1;
	else
		return -1;
#endif
}

static inline void ax_sem_destroy(ax_sem *sem)
{
	assert(sem);
#ifdef AX_OS_WIN
	CloseHandle(sem->hSem);
#else
	sem_destroy(&sem->sem);
#endif
}

#endif

