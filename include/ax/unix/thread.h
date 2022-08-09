#ifndef AX_UNIX_THREAD_H
#define AX_UNIX_THREAD_H

#include "ax/debug.h"
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#ifndef AX_THREAD_DEFINED
#define AX_THREAD_DEFINED
typedef struct ax_thread_st ax_thread;
#endif

#ifndef AX_THREAD_FUNC_F_DEFINED
#define AX_THREAD_FUNC_F_DEFINED
typedef uintptr_t (ax_thread_func_f)(void *arg);
#endif

struct ax_thread_st
{
	pthread_t thread;
};

static inline ax_thread ax_thread_self(void)
{
        return (ax_thread) { .thread = pthread_self() };
}

struct thread_argument_st
{
        void *arg;
        ax_thread_func_f *func;
};

static void *thread_function(void *arg)
{
        struct thread_argument_st ta = *(struct thread_argument_st *)arg;
	free(arg);
	return (void *)ta.func(ta.arg);
}

static inline int ax_thread_create(ax_thread_func_f *thread_func, void *arg, ax_thread *thread)
{
	struct thread_argument_st *ta = (struct thread_argument_st *)malloc(sizeof *ta);
	if (!ta)
		return -1;
	*ta = (struct thread_argument_st){ .arg = arg, .func = thread_func };
	
	return - !!pthread_create(&thread->thread, NULL, thread_function, ta);
}

static inline void ax_thread_sleep(ax_thread *thread, unsigned int millise)
{
	ax_assert(millise < UINT_MAX / 1000, "millise is too big");
	usleep(millise * 1000);
}

static inline void ax_thread_exit(uintptr_t retval)
{
	pthread_exit((void *)retval);
}

static inline int ax_thread_join(ax_thread *thread, uintptr_t *retval)
{
	void *retptr;
	int err = pthread_join(thread->thread, &retptr);
	if(err)
		return -1;
	if (retptr == PTHREAD_CANCELED)
		return -1;
	*retval = (uintptr_t)retptr;
	return 0;
}

static inline int ax_thread_detach(ax_thread *thread)
{
	return - !!pthread_detach(thread->thread);
}

static inline void ax_thread_yield(void)
{
	extern int sched_yield(void);
	sched_yield();
}

#endif
