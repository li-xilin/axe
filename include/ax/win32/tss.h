#ifndef AX_WIN32_TSS_H
#define AX_WIN32_TSS_H

#include <windows.h>
#include <process.h>
#include <errno.h>
#include <assert.h>

#ifndef AX_TSS_DEFINED
#define AX_TSS_DEFINED
typedef struct ax_tss_st ax_tss;
#endif

struct ax_tss_st
{
	DWORD dwKey;
};

static inline int ax_tss_init(ax_tss *key)
{
	assert(key);
	DWORD dwKey = TlsAlloc();
	if (dwKey == TLS_OUT_OF_INDEXES)
		return -1;
	key->dwKey = dwKey;
	return 0;
}

static inline void ax_tss_destroy(ax_tss *key)
{
	assert(key);
	TlsFree(key->dwKey);
}

static inline void *ax_tss_get(ax_tss *key)
{
	assert(key);
	return TlsGetValue(key->dwKey);
}

static inline int ax_tss_set(ax_tss *key, void *value)
{
	assert(key);
	return - !TlsSetValue(key->dwKey, value);
}

#endif
