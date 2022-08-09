#ifndef AX_UNIX_SEM_H
#define AX_UNIX_SEM_H

#define _BSD_SOURCE

#include <semaphore.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <time.h>

struct ax_sem_st
{
	 sem_t sem;
};

#ifndef AX_SEM_DEFINED
#define AX_SEM_DEFINED
typedef struct ax_sem_st ax_sem;
#endif

static inline int ax_sem_init(ax_sem *sem, size_t initial_value)
{
	assert(sem);
	return sem_init(&sem->sem, 0, initial_value);
}

static inline int ax_sem_post(ax_sem *sem)
{
	assert(sem);
	return sem_post(&sem->sem);
}

static inline int ax_sem_wait(ax_sem *sem, uint32_t millise)
{
	assert(sem);

	if (millise < 0)
		return sem_wait(&sem->sem);

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

	int ret;
	while ((ret = sem_timedwait(&sem->sem, &spec)) == -1 && errno == EINTR)
		continue;
	if (!ret)
		return 0;
	if (errno == ETIMEDOUT)
		return 1;
	else
		return -1;
}

static inline void ax_sem_destroy(ax_sem *sem)
{
	assert(sem);
	sem_destroy(&sem->sem);
}

#endif
