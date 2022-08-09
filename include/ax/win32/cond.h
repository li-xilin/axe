#ifndef AX_WIN32_COND_H
#define AX_WIN32_COND_H

// #define _WIN32_WINNT 0x0600

#include "mutex.h"
#include <synchapi.h>
#include <stdint.h>
#include <assert.h>

struct ax_cond_st
{
	CONDITION_VARIABLE CondVar;
};

#define AX_COND_INIT { .CondVar = CONDITION_VARIABLE_INIT }

#ifndef AX_COND_DEFINED
#define AX_COND_DEFINED
typedef struct ax_cond_st ax_cond;
#endif

static inline int ax_cond_init(ax_cond *cond)
{
	assert(cond);
	InitializeConditionVariable(&cond->CondVar);
	return 0;
}

static inline int ax_cond_sleep(ax_cond *cond, ax_mutex *mutex, int millise)
{
	assert(cond);
	assert(mutex);
	return - !SleepConditionVariableCS(&cond->CondVar, &mutex->section, millise < 0 ? INFINITE : millise);
}

static inline int ax_cond_wake(ax_cond *cond)
{
	assert(cond);
	WakeConditionVariable(&cond->CondVar);
	return 0;
}

static inline int ax_cond_wake_all(ax_cond *cond)
{
	assert(cond);
	WakeAllConditionVariable(&cond->CondVar);
	return 0;
}

static inline void ax_cond_destroy(ax_cond *cond)
{
	assert(cond);
}

#endif
