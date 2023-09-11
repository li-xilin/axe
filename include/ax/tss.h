#ifndef AX_TSS_H
#define AX_TSS_H

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

#ifndef AX_OS_WIN
static inline void _ax_tss_free_cb(void *ptr) { }
#endif

static inline int ax_tss_init(ax_tss *key)
{
	assert(key);
#ifdef AX_OS_WIN
	DWORD dwKey = TlsAlloc();
	if (dwKey == TLS_OUT_OF_INDEXES)
		return -1;
	key->dwKey = dwKey;
#else
	return pthread_key_create(&key->key, &_ax_tss_free_cb);
#endif
	return 0;
}

static inline void ax_tss_destroy(ax_tss *key)
{
	assert(key);

#ifdef AX_OS_WIN
	TlsFree(key->dwKey);
#else
	pthread_key_delete(key->key);
#endif
}

static inline void *ax_tss_get(ax_tss *key)
{
	assert(key);
#ifdef AX_OS_WIN
	return TlsGetValue(key->dwKey);
#else
	return pthread_getspecific(key->key);
#endif
}

static inline int ax_tss_set(ax_tss *key, void *value)
{
	assert(key);
#ifdef AX_OS_WIN
	if (!TlsSetValue(key->dwKey, value))
		return -1;
#else
	if (pthread_setspecific(key->key, value)) {
		return -1;
	}
#endif
	return 0;
}

#endif

