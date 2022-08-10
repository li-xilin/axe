#include "ax/tpool.h"
#include "ax/thread.h"
#include "ax/mutex.h"
#include "ax/cond.h"

#include <stdlib.h>
#include <string.h>

struct ax_tpool_work_st
{
	ax_tpool_worker_f *func;
	void *arg;
	struct ax_tpool_work_st *next;
};

struct ax_tpool_st
{
	ax_tpool_work *work_first, *work_last;
	ax_cond work_cond, working_cond;
	size_t working_cnt, thread_cnt;
	ax_mutex work_mutex;
	bool stop;
};

static ax_tpool_work *ax_tpool_work_create(ax_tpool_worker_f *func, void *arg)
{
	assert(func);
	ax_tpool_work *work;
	work = malloc(sizeof *work);
	if (!work)
		return NULL;
	work->func = func;
	work->arg  = arg;
	work->next = NULL;
	return work;
}

static void ax_tpool_work_destroy(ax_tpool_work *work)
{
	if (!work)
		return;
	free(work);
}

static ax_tpool_work *ax_tpool_work_get(ax_tpool *tpool)
{
	ax_tpool_work *work;

	if (tpool == NULL)
		return NULL;

	work = tpool->work_first;
	if (work == NULL)
		return NULL;

	if (work->next == NULL) {
		tpool->work_first = NULL;
		tpool->work_last  = NULL;
	} else {
		tpool->work_first = work->next;
	}

	return work;
}

static uintptr_t ax_tpool_worker(void *arg)
{
	ax_tpool      *tpool = arg;
	ax_tpool_work *work;

	while (1) {
		ax_mutex_lock(&(tpool->work_mutex));

		while (tpool->work_first == NULL && !tpool->stop)
			ax_cond_sleep(&(tpool->work_cond), &(tpool->work_mutex), -1);

		if (tpool->stop)
			break;

		work = ax_tpool_work_get(tpool);
		// tpool->working_cnt++;
		ax_mutex_unlock(&(tpool->work_mutex));

		if (work != NULL) {
			work->func(work->arg);
			ax_tpool_work_destroy(work);
		}

		ax_mutex_lock(&(tpool->work_mutex));
		tpool->working_cnt--;
		if (!tpool->stop && tpool->working_cnt == 0 && tpool->work_first == NULL)
			ax_cond_wake(&(tpool->working_cond));
		ax_mutex_unlock(&(tpool->work_mutex));
	}

	tpool->thread_cnt--;
	ax_cond_wake(&(tpool->working_cond));
	ax_mutex_unlock(&(tpool->work_mutex));
	return 0;
}

ax_tpool *ax_tpool_create(size_t num)
{
	ax_tpool   *tpool;
	ax_thread  thread;
	size_t     i;

	memset(&thread, 0, sizeof thread);

	if (num == 0)
		num = 2;

	tpool = calloc(1, sizeof(*tpool));
	tpool->thread_cnt = num;
	tpool->stop = false;

	ax_mutex_init(&tpool->work_mutex);
	ax_cond_init(&tpool->work_cond);
	ax_cond_init(&tpool->working_cond);

	tpool->work_first = NULL;
	tpool->work_last  = NULL;

	for (i=0; i<num; i++) {
		if (ax_thread_create(ax_tpool_worker, tpool, &thread))
			continue;
		ax_thread_detach(&thread);
	}

	return tpool;
}

void ax_tpool_destroy(ax_tpool *tpool)
{
	assert(tpool);
	ax_tpool_work *work;
	ax_tpool_work *work2;

	ax_mutex_lock(&(tpool->work_mutex));
	work = tpool->work_first;
	while (work != NULL) {
		work2 = work->next;
		ax_tpool_work_destroy(work);
		work = work2;
	}
	tpool->stop = true;
	ax_cond_wake_all(&(tpool->work_cond));
	ax_mutex_unlock(&(tpool->work_mutex));

	ax_tpool_wait(tpool);

	ax_mutex_destroy(&(tpool->work_mutex));
	ax_cond_destroy(&(tpool->work_cond));
	ax_cond_destroy(&(tpool->working_cond));

	free(tpool);
}

bool ax_tpool_add_work(ax_tpool *tpool, ax_tpool_worker_f *func, void *arg)
{
	assert(tpool);
	assert(func);
	ax_tpool_work *work;

	work = ax_tpool_work_create(func, arg);
	if (work == NULL)
		return false;

	ax_mutex_lock(&(tpool->work_mutex));

	tpool->working_cnt++;

	if (tpool->work_first == NULL) {
		tpool->work_first = work;
		tpool->work_last  = tpool->work_first;
	} else {
		tpool->work_last->next = work;
		tpool->work_last       = work;
	}

	ax_cond_wake_all(&(tpool->work_cond));
	ax_mutex_unlock(&(tpool->work_mutex));

	return true;
}

void ax_tpool_wait(ax_tpool *tpool)
{
	assert(tpool);
	ax_mutex_lock(&(tpool->work_mutex));
	while (true) {
		if ((!tpool->stop && tpool->working_cnt != 0) || (tpool->stop && tpool->thread_cnt != 0)) {
			ax_cond_sleep(&(tpool->working_cond), &(tpool->work_mutex), -1);
		} else {
			break;
		}
	}
	ax_mutex_unlock(&(tpool->work_mutex));
}

