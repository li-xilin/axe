#ifndef AX_WIN32_SEM_H
#define AX_WIN32_SEM_H

#include <winbase.h>
#include <synchapi.h>
#include <stdint.h>
#include <assert.h>

struct ax_sem_st
{
	HANDLE hSem;
};

#ifndef AX_SEM_DEFINED
#define AX_SEM_DEFINED
typedef struct ax_sem_st ax_sem;
#endif

static inline int ax_sem_init(ax_sem *sem, size_t initial_value)
{
	assert(sem);
	SECURITY_ATTRIBUTES Attr = {
		.nLength = sizeof Attr,
		.lpSecurityDescriptor = NULL,
		.bInheritHandle = TRUE,
	};
	return - !CreateSemaphoreA(&Attr, initial_value, ULONG_MAX, NULL);
}

static inline int ax_sem_post(ax_sem *sem)
{
	assert(sem);
	return - !!ReleaseSemaphore(sem->hSem, 1, NULL);
}

static inline int ax_sem_wait(ax_sem *sem, int millise)
{
	assert(sem);
	DWORD ret = WaitForSingleObject(sem->hSem, millise < 0 ? INFINITE : millise);
	if (ret == WAIT_FAILED)
		return -1;
	if (ret == WAIT_TIMEOUT)
		return 1;
	return 0;
}

static inline void ax_sem_destroy(ax_sem *sem)
{
	assert(sem);
	CloseHandle(sem->hSem);
}

#endif
