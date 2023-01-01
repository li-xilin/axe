#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "reactor_type.h"
#include "ax/socket.h"
#include "ax/event.h"
#include "ax/link.h"

void ax_event_set(ax_event *e, ax_socket fd, short ev_flags, ax_event_cb_f *cb, void *arg){
	assert(e != NULL);

	e->ev_flags = ev_flags;
	e->cb = cb;
	e->arg = arg;
	e->fd = fd;
	e->timerheap_idx = E_OUT_OF_TIMERHEAP;
	ax_link_init_empty(&e->event_link);
	ax_link_init_empty(&e->pending_link);
	ax_link_init_empty(&e->hash_link);
}

bool ax_event_in_use(const ax_event *e){
	assert(e != NULL);

	return e->ev_flags & AX_EV_REACTING;
}
