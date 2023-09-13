#ifndef AX_TSS_H
#define AX_TSS_H

#include "detect.h"
#include <errno.h>
#include <assert.h>

#ifdef AX_OS_WIN
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

#ifndef AX_TSS_DEFINED
#define AX_TSS_DEFINED
typedef struct ax_tss_st ax_tss;
#endif

struct ax_tss_st
{
#ifdef AX_OS_WIN
	DWORD dwKey;
#else
	pthread_key_t key;
#endif
};

typedef void (ax_tss_free_f)(void *ptr);

static inline int ax_tss_create(ax_tss *key, ax_tss_free_f *free_cb)
{
	assert(key);
#ifdef AX_OS_WIN
	extern int ax_tss_create_win32(ax_tss *key, ax_tss_free_f *free_cb);
	if (ax_tss_create_win32(key, free_cb))
		return -1;
#else
	return pthread_key_create(&key->key, free_cb);
#endif
	return 0;
}

static inline void ax_tss_remove(ax_tss *key)
{
	assert(key);

#ifdef AX_OS_WIN
	extern void ax_tss_remove_win32(ax_tss *key);
	ax_tss_remove_win32(key);
#else
	pthread_key_delete(key->key);
#endif
}

static inline void *ax_tss_get(ax_tss *key)
{
	assert(key);
#ifdef AX_OS_WIN
	extern void *ax_tss_get_win32(ax_tss *key);
	return ax_tss_get_win32(key);
#else
	return pthread_getspecific(key->key);
#endif
}

static inline int ax_tss_set(ax_tss *key, void *value)
{
	assert(key);
#ifdef AX_OS_WIN
	extern int ax_tss_set_win32(ax_tss *key, void *value);
	if (ax_tss_set_win32(key, value))
		return -1;
#else
	if (pthread_setspecific(key->key, value)) {
		return -1;
	}
#endif
	return 0;
}

#endif

