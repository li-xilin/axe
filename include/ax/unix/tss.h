#ifndef AX_UNIX_TSS_H
#define AX_UNIX_TSS_H

#include <assert.h>
#include <pthread.h>

#ifndef AX_TSS_DEFINED
#define AX_TSS_DEFINED
typedef struct ax_tss_st ax_tss;
#endif

struct ax_tss_st
{
	pthread_key_t key;
};

static inline void _ax_tss_free_cb(void *ptr)
{
}

static inline int ax_tss_init(ax_tss *key)
{
	assert(key);
	return pthread_key_create(&key->key, &_ax_tss_free_cb);
}

static inline void ax_tss_destroy(ax_tss *key)
{
	assert(key);
	pthread_key_delete(key->key);
}

static inline void *ax_tss_get(ax_tss *key)
{
	assert(key);
	return pthread_getspecific(key->key);
}

static inline int ax_tss_set(ax_tss *key, void *value)
{
	assert(key);
	return pthread_setspecific(key->key, value);
}

#endif

