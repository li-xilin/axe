#include "ax/tss.h"
#include "ax/errno.h"
#include "ax/detect.h"
#include "ax/link.h"

#ifdef AX_OS_WIN

#include <windows.h>
#include <malloc.h>

#define TLS_MAXIIUM_AVAILABLE 1088

typedef struct {
	ax_link Link;
	DWORD dwKey;
	ax_tss_free_f *pfnFreeProc;
} TLS_NODE, *PTLS_NODE;

static struct _TLS_CONTEXT {
	ax_link TlsList;
	CRITICAL_SECTION Lock;
} *sg_TlsContext = NULL;


static int TryInitTlsContext(void)
{
	if (sg_TlsContext)
		return 0;
	struct _TLS_CONTEXT *ctx = malloc(sizeof *ctx);
	if (!ctx)
		return -1;
	InitializeCriticalSection(&ctx->Lock);
	ax_link_init(&ctx->TlsList);
	if (InterlockedCompareExchangePointer(&sg_TlsContext, ctx, NULL))
		free(ctx);
	return 0;
}

int ax_tss_create_win32(ax_tss *key, ax_tss_free_f *free_cb)
{
	int rc = -1;

	if (TryInitTlsContext())
		return rc;

	EnterCriticalSection(&sg_TlsContext->Lock);
	PTLS_NODE pNode = malloc(sizeof *pNode);
	if (!pNode)
		goto unlock;

	DWORD dwKey = TlsAlloc();
	if (dwKey == TLS_OUT_OF_INDEXES) {
		free(pNode);
		goto unlock;
	}
	key->dwKey = dwKey;
	pNode->dwKey = dwKey;
	pNode->pfnFreeProc = free_cb;
	ax_link_add_back(&pNode->Link, &sg_TlsContext->TlsList);

	rc = 0;
unlock:
	LeaveCriticalSection(&sg_TlsContext->Lock);
	return rc;
}

void ax_tss_remove_win32(ax_tss *key)
{
	if (!sg_TlsContext)
		return;
	EnterCriticalSection(&sg_TlsContext->Lock);
	TlsFree(key->dwKey);
	ax_link *cur;
	ax_link_foreach(cur, &sg_TlsContext->TlsList) {
		PTLS_NODE pNode = ax_link_entry(cur, TLS_NODE, Link);
		if (pNode->dwKey == key->dwKey) {
			ax_link_del(cur);
			free(pNode);
			goto out;
		}
	}
out:
	LeaveCriticalSection(&sg_TlsContext->Lock);
}

void __ax_tss_free_all_win32(void)
{
	if (!sg_TlsContext)
		return;

	DWORD dwTableLen = 0;
	struct {
		LPVOID pSlotValue;
		ax_tss_free_f *pfnFree;
	} aNodeTable[TLS_MAXIIUM_AVAILABLE];

	EnterCriticalSection(&sg_TlsContext->Lock);
	ax_link *cur;
	ax_link_foreach(cur, &sg_TlsContext->TlsList) {
		PTLS_NODE pNode = ax_link_entry(cur, TLS_NODE, Link);
		LPVOID pSlotValue = TlsGetValue(pNode->dwKey);
		if (!pSlotValue)
			continue;
		aNodeTable->pfnFree = pNode->pfnFreeProc;
		aNodeTable->pSlotValue = pSlotValue;
		dwTableLen++;
	}
	LeaveCriticalSection(&sg_TlsContext->Lock);

	for (int i = 0; i < dwTableLen; i++)
		aNodeTable[i].pfnFree((ax_tss *)aNodeTable[i].pSlotValue);
}

void *ax_tss_get_win32(ax_tss *key)
{
	void *val;;
	if (!(val = TlsGetValue(key->dwKey)))
		ax_error_occur();
	return val;
}

int ax_tss_set_win32(ax_tss *key, void *value)
{
	if (!TlsSetValue(key->dwKey, value)) {
		ax_error_occur();
		return -1;
	}
	return 0;
}

#else

int __ax_tss_unused = 0;

#endif

