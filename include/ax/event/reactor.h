#ifndef AX_EVENT_REACTOR_H
#define AX_EVENT_REACTOR_H

#include "util.h"
#include "event.h"
#include "signal.h"
#include "../mutex.h"

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

ax_reactor *ax_reactor_create();

void ax_reactor_exit(ax_reactor *r);

void ax_reactor_destroy(ax_reactor *r);

int ax_reactor_add(ax_reactor *r, ax_event *e);

int ax_reactor_modify(ax_reactor *r, ax_event *e);

void ax_reactor_add_to_pending(ax_reactor *r, ax_event *e, short res_flags);

int ax_reactor_remove(ax_reactor *r, ax_event *e);

bool ax_reactor_empty(ax_reactor *r);

void ax_reactor_clear(ax_reactor *r);

void ax_reactor_loop(ax_reactor *r, struct timeval *timeout, int flags);

#endif
