#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ax/event/event.h"
#include "ax/event/util.h"
#include "ax/link.h"

void ax_event_set(ax_event *e, ax_socket fd, short ev_flags, event_callback callback, void *arg){
	assert(e != NULL);

	e->ev_flags = ev_flags;
	e->callback = callback;
	e->callback_arg = arg;
	e->fd = fd;
	e->timerheap_idx = E_OUT_OF_TIMERHEAP;
	ax_link_init_empty(&e->event_link);
	ax_link_init_empty(&e->pending_link);
	ax_link_init_empty(&e->hash_link);
}

bool ax_event_in_use(const ax_event *e){
	assert(e != NULL);

	return e->ev_flags & E_IN_REACTOR;
}