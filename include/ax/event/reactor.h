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

struct ax_event_ht_st;

/* exits after having processed at least one active event */
#define REACTOR_ONCE 0x01

struct ax_reactor_st {
	/* events registered */
	ax_link event_list;
	/* active events waiting for being processed */
	ax_link pending_list;
	
	/* policy specified data, usually the back pointer of the reactor. */
	void * polling_data;

	/* event hash table */
	struct ax_event_ht_st *eht;

	/* 
	 * Lock to avoid race conditions.
	 * if null we assume this reactor is in single-threaded environment.
	 */
	ax_mutex lock;

	/* Unnamed pipe used to wake up the polling thread. */
	ax_socket pipe[2];
	ax_event pe;

	/* Signal event internal data */
	struct signal_internal * psi;
	/* Unnamed pipe used to tell the reactor that a signal has occurred. */
	ax_socket sig_pipe[2];
	ax_event * sig_pe;

	/* Timer event internal data */
	struct timerheap_internal * pti;
	/* Unnamed pipe used to tell the reactor that a timer has expired. */
	ax_socket timer_pipe[2];
	ax_event * timer_pe;

	/* Indicates whether we should stop polling. */
	int out;
};

void ax_reactor_exit(ax_reactor * r);

int ax_reactor_init(ax_reactor * r);

void ax_reactor_destroy(ax_reactor * r);

int ax_reactor_add(ax_reactor * r, ax_event * e);

int ax_reactor_modify(ax_reactor * r, ax_event * e);

int ax_reactor_add_to_pending(ax_reactor * r, ax_event * e, short res_flags);

int ax_reactor_remove(ax_reactor * r, ax_event * e);

bool ax_reactor_empty(ax_reactor * r);

void ax_reactor_clear(ax_reactor * r);

void ax_reactor_loop(ax_reactor * r, struct timeval * timeout, int flags);

#endif
