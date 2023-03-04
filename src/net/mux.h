#ifndef MUX_H
#define MUX_H

#include "event_ht.h"
#include "ax/reactor.h"
#include "ax/mutex.h"

typedef void mux_pending_cb(ax_socket fd, short flags, void *arg);

typedef struct mux_st mux;

mux *mux_init(void);
int mux_add(mux *mux, ax_socket fd, short flags);
int mux_mod(mux *mux, ax_socket fd, short flags);
void mux_del(mux *mux, ax_socket fd, short flags);
int mux_poll(mux *mux, ax_mutex *lock, struct timeval * timeout, mux_pending_cb *cb, void *arg);
void mux_free(mux *mux);

#endif
