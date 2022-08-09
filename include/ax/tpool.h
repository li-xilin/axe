#ifndef AX_TPOOL_H
#define AX_TPOOL_H

#include <stdbool.h>
#include <stddef.h>


#ifndef AX_TPOOL_WORK_DEFINED
#define AX_TPOOL_WORK_DEFINED
typedef struct ax_tpool_work_st ax_tpool_work;
#endif

#ifndef AX_TPOOL_DEFINED
#define AX_TPOOL_DEFINED
typedef struct ax_tpool_st ax_tpool;
#endif

typedef void ax_tpool_worker_f(void *arg);

ax_tpool *ax_tpool_create(size_t num);
void ax_tpool_destroy(ax_tpool *tpool);

bool ax_tpool_add_work(ax_tpool *tpool, ax_tpool_worker_f *func, void *arg);
void ax_tpool_wait(ax_tpool *tpool);

#endif
