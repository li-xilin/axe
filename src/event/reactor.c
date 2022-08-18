/*
 * Copyright (c) 2014 Xinjing Chow
 */

#include "polling.h"
#include "event_ht.h"
#include "timer.h"
#include "evsignal.h"
#include "ax/event/reactor.h"
#include "ax/event/event.h"
#include "ax/event/skutil.h"

#include "ax/log.h"
#include "ax/link.h"

#ifdef AX_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

/*
 * Initialize the reactor.
 * @r: the reactor to be initialized.
 * @policy_name: the name of polling policy.
 * @in_mt: indicates whether this application is used in multithreaded environment.
 * @handle_sig: indicates whether the reactor handles signal events.
 * @handle_timer: indicates whether the reactor handles timer events.
 */
static int reactor_init_(ax_reactor * r, int in_mt, int handle_sig, int handle_timer);

/*
 * Dequeue a event from event list.
 * Return: the dequeued event or null if the list is empty.
 * @r: the reactor.
 */
static ax_event * reactor_dequeue_event(ax_reactor * r);

/*
 * Initialize signal events handling mechanism.
 * Return: 0 on success, -1 on failure.
 * @r: the related reactor.
 */
static int reacotr_signal_init(ax_reactor * r);

/*
 * Initialize timer events handling mechanism.
 * Return: 0 on success, -1 on failure.
 * @r: the related reactor.
 */
int reactor_timer_init(ax_reactor * r);

/*
 * The signal event callback used to call corresponding signal event callbacks.
 */
static void reactor_signal_callback(ax_socket fd, short res_flags, void *arg);

static void reactor_signal_callback(ax_socket fd, short res_flags, void *arg) {
	ax_reactor * r;
	ax_event * e;
	struct signal_internal * psi;
	int sig;

	sig = 0;
	r = arg;
	psi = r->psi;
	assert(r != NULL && psi != NULL);

	while (read(fd, &sig, 1) > 0) {
		e = psi->sigevents[sig];

		assert(e != NULL && e->callback != NULL);

		e->callback(e->fd, E_SIGNAL, e->callback_arg);
	}
}

static void reactor_handle_timer(ax_reactor * r) {
	ax_event * e;

	assert(r != NULL);

	while (timerheap_top_expired(r)) {
		e = timerheap_get_top(r);

		assert(e != NULL);

		if (e->ev_flags & E_ONCE && e->timerheap_idx != E_OUT_OF_TIMERHEAP) {
			if (timerheap_remove_event(r, e)  == -1) {
				ax_perror("failed to remove timer event from the heap");
			} else {
				e->ev_flags &= ~E_IN_REACTOR;
			}
		} else if (!(e->ev_flags & E_ONCE) && e->timerheap_idx != E_OUT_OF_TIMERHEAP) {
			timerheap_reset_timer(r, e);
		}

		ax_mutex_unlock(&r->lock);
		e->callback(e->fd, e->res_flags, e->callback_arg);
		ax_mutex_lock(&r->lock);
	}
}
/*
 * The default callback got called after reactor being waked up.
 */
static void reactor_waked_up(ax_socket fd, short res_flags, void *arg) {
	//ax_perror("woke up.");
	int n;
	char buf[1024];
	while ((n = read(fd, buf, sizeof(buf))) > 0);
}

/*
 * Wake up the polling thread.
 * @r: the reactor to wake up
 */
static void reactor_wake_up(ax_reactor * r) {
	assert(r != NULL);

	char octet = 0;
	int  n = 0;

	n = write(r->pipe[1], &octet, sizeof(octet));
	ax_unused(n);
	assert(n > 0);
}

void reactor_get_out(ax_reactor * r) {
	ax_mutex_lock(&r->lock);
	r->out = 1;
	reactor_wake_up(r);
	ax_mutex_unlock(&r->lock);
}

static int reactor_init_(ax_reactor * r, int in_mt, int handle_sig, int handle_timer) {
	assert(r != NULL);

	memset(r, 0, sizeof(ax_reactor));
	r->eht = malloc(sizeof(struct ax_event_ht_st));
	if (!r->eht)
		return -1;
	if (event_ht_init(r->eht, 0.5) == -1) {
		ax_perror("failed to initialize the hash table of events.");
		exit(1);
	}

	ax_link_init(&r->event_list);
	ax_link_init(&r->pending_list);

	/* Policy internal initialization. */
	r->polling_data = polling_init(r);
	if (r->polling_data == NULL) {
		ax_perror("failed to initialize polling");
		exit(1);
	}

	if (in_mt) { //TODO
		/* We only use lock in multithreaded environment to reduce overhead. */
	}

	/* If the lock is null, locking functions will be no-ops */
	ax_mutex_init(&r->lock);
	r->out = 0;
	if (ax_skutil_create_pipe(r->pipe) == -1) {
		ax_perror("failed to create informing pipe.");
		exit(1);
	}
	/* set the pipe to nonblocking mode */
	if (ax_skutil_set_nonblocking(r->pipe[0]) < 0 || ax_skutil_set_nonblocking(r->pipe[1]) < 0) {
		ax_perror("failed to set the pipe to nonblocking mode.");
		exit(1);
	}

	//set up the informer
	event_set(&r->pe, r->pipe[0], E_READ, reactor_waked_up, NULL);
	reactor_add_event(r, &r->pe);

	if (handle_sig) {
		//set up signal events handling
		if (reacotr_signal_init(r) == -1) {
			ax_perror("failed to initialize signal handling.");
			reactor_destroy(r);
		}
	}
	if (handle_timer) {
		//set up timer events handling
		if (reactor_timer_init(r) == -1) {
			ax_perror("failed to initialize signal handling.");
			reactor_destroy(r);
		}
	}
	return 0;
}

int reactor_init(ax_reactor * r) {
	return reactor_init_(r, 0, 0, 0);
}
int reactor_init_with_mt(ax_reactor * r) {
	return reactor_init_(r, 1, 0, 0);
}
int reactor_init_with_signal(ax_reactor * r) {
	return reactor_init_(r, 0, 1, 0);
}

int reactor_init_with_timer(ax_reactor * r) {
	return reactor_init_(r, 0, 0, 1);
}

int reactor_init_with_signal_timer(ax_reactor * r) {
	return reactor_init_(r, 0, 1, 1);
}

int reactor_init_with_mt_signal(ax_reactor * r) {
	return reactor_init_(r, 1, 1, 0);
}

int reactor_init_with_mt_timer(ax_reactor * r) {
	return reactor_init_(r, 1, 0, 1);
}

int reactor_init_with_mt_signal_timer(ax_reactor * r) {
	return reactor_init_(r, 1, 1, 1);
}

/*
 * Frees up event_list
 * @r: the reactor
 */
static void reactor_free_events(ax_reactor * r) {
	ax_event * e = NULL;

	while ((e = reactor_dequeue_event(r))) {
		if (polling_del(r, e->fd, e->ev_flags) == -1) {
			ax_perror("failed to remove the event[%d] from the reactor.", e->fd);
		}
		ax_link_del(&e->event_link);

		event_ht_delete(r->eht, e);

		ax_link_del(&e->pending_link);

		e->rc = NULL;
	}
}

/*
 * Frees up event hash table
 * @r: the reactor
 */
static void reactor_free_hash(ax_reactor * r) {
	event_ht_free(r->eht);
}

void reactor_clean_events(ax_reactor * r) {
	ax_mutex_lock(&r->lock);
	reactor_free_events(r);
	if (r->pti) {
		timerheap_clean_events(r);
	}
	ax_mutex_unlock(&r->lock);
	/* remove all events except this informing pipe */
	event_set(&r->pe, r->pipe[0], E_READ, reactor_waked_up, NULL);
	if (reactor_add_event(r, &r->pe) == -1) {
		ax_perror("failed to add informing pipe event back to reactor");
	}
}

void reactor_destroy(ax_reactor * r) {
	ax_mutex_lock(&r->lock);
	ax_pdebug("free up event_list.");
	//frees up event_list
	reactor_free_events(r);

	ax_pdebug("free up hash table.");
	//frees up hash table
	reactor_free_hash(r);

	ax_pdebug("free up polling policy.");
	//free up polling policy
	polling_destroy(r);

	//close the pipe
	ax_skutil_close_fd(r->pipe[0]);
	ax_skutil_close_fd(r->pipe[1]);

	//close the signal event handling stuff
	if (r->psi) {
		ax_skutil_close_fd(r->sig_pipe[0]);
		ax_skutil_close_fd(r->sig_pipe[1]);
		signal_internal_restore_all(r);
		free(r->psi);
		free(r->sig_pe);
		r->psi = NULL;
		r->sig_pe = NULL;
	}

	//close the timer event handling stuff
	if (r->pti) {
		ax_skutil_close_fd(r->sig_pipe[0]);
		ax_skutil_close_fd(r->sig_pipe[1]);
		timerheap_destroy(r);
		free(r->pti);
		r->pti = NULL;
	}
	ax_mutex_unlock(&r->lock);
	//free up the lock
	ax_mutex_destroy(&r->lock);

	free(r->eht);
}

static int reacotr_signal_init(ax_reactor * r) {
	if ((r->psi = signal_internal_init(r)) == NULL) {
		ax_perror("failed on signal_internal_init");
		exit(1);
	}

	if (ax_skutil_create_pipe(r->sig_pipe) == -1) {
		ax_perror("failed to create signal informing pipe.");
		exit(1);
	}

	/* set the pipe to nonblocking mode */
	if (ax_skutil_set_nonblocking(r->sig_pipe[0]) < 0 || ax_skutil_set_nonblocking(r->sig_pipe[1]) < 0) {
		ax_perror("failed to set the pipe to nonblocking mode.");
		exit(1);
	}

	if ((r->sig_pe = event_new(r->sig_pipe[0], E_READ, reactor_signal_callback, r)) == NULL) {
		ax_perror("failed to create event for signal events handling");
		exit(1);
	}

	reactor_add_event(r, r->sig_pe);

	return (0);
}

/*
 * Initialize timer events handling mechanism.
 * Return: 0 on success, -1 on failure.
 * @r: the related reactor.
 */
int reactor_timer_init(ax_reactor * r) {
	if ((r->pti = timerheap_internal_init(r)) == NULL) {
		ax_perror("failed on timerheap_internal_init: %s", strerror(errno));
		return (-1);
	}
	return (0);
}

int reactor_add_event(ax_reactor * r, ax_event * e) {
	assert(r != NULL && e != NULL);

	ax_mutex_lock(&r->lock);

	if (e->ev_flags & E_SIGNAL || e->ev_flags & E_TIMEOUT) {
		/* Signal or timer event registration */
		if (e->ev_flags & E_SIGNAL && e->ev_flags & E_TIMEOUT) {
			ax_mutex_unlock(&r->lock);
			ax_perror("Inlivad flags[%d], E_SIGNAL and E_TIMEOUT are mutually exclusive.", e->ev_flags);
			return (-1);
		}else if (e->ev_flags & E_SIGNAL) {
			if (signal_internal_register(r, (int)e->fd, e) == -1) {
				ax_mutex_lock(&r->lock);
				ax_perror("failed to register event for signal[%d]", (int)e->fd);
				return (-1);
			}
		}else{//E_TIMEOUT
			int size = r->pti->size;
			ax_unused(size);
			assert(e->timerheap_idx == E_OUT_OF_TIMERHEAP);
			if (timerheap_add_event(r, e) == -1) {
				ax_mutex_unlock(&r->lock);
				ax_perror("failed to register timer event");
				return (-1);
			}
			assert(r->pti->size == size + 1);
			assert(e->timerheap_idx != E_OUT_OF_TIMERHEAP);
		}
	}else{
		/* Normal I/O event registration. */
		if (e->event_link.prev || e->event_link.next) {
			ax_mutex_unlock(&r->lock);
			/*
			 * This event is already in the reactor.
			 * Assume every event only can be in one reactor.
			 */
			ax_perror("event already in the reactor");
			return (-1);
		}
		//ax_perror("Adding a event [fd %d]", e->fd);
		if (polling_add(r, e->fd, e->ev_flags) == -1) {
			ax_mutex_unlock(&r->lock);
			ax_perror("failed to add the event[%d] to the reactor.", e->fd);
			return (-1);
		}
		//ax_perror("Added a event [fd %d]", e->fd);
		ax_link_add_back(&e->event_link, &r->event_list);

		event_ht_insert(r->eht, e, e->fd);
	}

	e->rc = r;
	e->ev_flags |= E_IN_REACTOR;

	//The polling thread might be sleeping indefinitely, wake it up.
	reactor_wake_up(r);

	ax_mutex_unlock(&r->lock);

	return (0);
}

int reactor_modify_events(ax_reactor * r, ax_event * e) {
	assert(r != NULL && e != NULL);

	ax_mutex_lock(&r->lock);

	if (e->ev_flags & E_SIGNAL || e->ev_flags & E_TIMEOUT) {
		ax_perror("Modification of signal or timer event is not supported");
		return (-1);
	}else{
		//ax_perror("Modifying the event [fd %d]", e->fd);
		if (polling_mod(r, e->fd, e->ev_flags) == -1) {
			ax_mutex_unlock(&r->lock);
			ax_perror("failed to modify the event[%d] in the reactor.", e->fd);
			return (-1);
		}
		//ax_perror("Modifying the event [fd %d]", e->fd);
	}

	e->rc = r;
	e->ev_flags |= E_IN_REACTOR;

	//The polling thread might be sleeping indefinitely, wake it up.
	reactor_wake_up(r);

	ax_mutex_unlock(&r->lock);

	return (0);
}

int reactor_add_to_pending(ax_reactor * r, ax_event * e, short res_flags) {
	assert(r != NULL && e != NULL);

	e->res_flags = res_flags;

	if (e->pending_link.prev || e->pending_link.next) {
		/*
		 * This event is alrady in the pending list.
		 * Assume every event only can be in one reactor.
		 */
		return (-1);
	}

	ax_link_add_back(&e->pending_link, &r->pending_list);

	return (0);
}

int reactor_remove_event(ax_reactor * r, ax_event * e) {
	assert(r != NULL && e != NULL);

	ax_mutex_lock(&r->lock);
	if (e->ev_flags & E_SIGNAL || e->ev_flags & E_TIMEOUT) {
		/* Signal or timer event unregistration */
		if (e->ev_flags & E_SIGNAL && e->ev_flags & E_TIMEOUT) {
			ax_mutex_unlock(&r->lock);
			ax_perror("Inlivad flags[%d], E_SIGNAL and E_TIMEOUT are mutually exclusive.", e->ev_flags);
			return (-1);
		}else if (e->ev_flags & E_SIGNAL) {
			if (signal_internal_unregister(r, (int)e->fd) == -1) {
				ax_mutex_unlock(&r->lock);
				ax_perror("failed to unregister event for signal[%d]", (int)e->fd);
				return (-1);
			}
		}else{//E_TIMEOUT
			if (e->timerheap_idx != E_OUT_OF_TIMERHEAP && timerheap_remove_event(r, e) == -1) {
				ax_mutex_unlock(&r->lock);
				ax_perror("failed to unregister time event");
				return (-1);
			}
			assert(e->timerheap_idx == E_OUT_OF_TIMERHEAP);
		}
	}else{
		/* Normal I/O event unregistration. */
		if (e->event_link.prev == NULL || e->event_link.next == NULL) {
			ax_mutex_unlock(&r->lock);
			ax_perror("The event is not in the reactor.");
			/*
			 * This event is not in the reactor.
			 * Assume every event only can be in one reactor.
			 */
			return (-1);
		}
		//ax_perror("Removing a event [fd %d]", e->fd);
		if (polling_del(r, e->fd, e->ev_flags) == -1) {
			ax_mutex_unlock(&r->lock);

			ax_perror("failed to remove the event[%d] from the reactor.", e->fd);
			return (-1);
		}
		//ax_perror("Removed a event [fd %d]", e->fd);

		ax_link_del(&e->event_link);

		event_ht_delete(r->eht, e);

		ax_link_del(&e->pending_link);
	}

	e->rc = NULL;
	e->ev_flags &= ~E_IN_REACTOR;

	//The polling thread might be sleeping indefinitely, wake it up.
	reactor_wake_up(r);

	ax_mutex_unlock(&r->lock);

	return (0);
}

/*
 * Dequeue a event from pending list.
 * Return: the dequeued event or null if the list is empty.
 * @r: the reactor.
 */
static ax_event * reactor_dequeue_pending(ax_reactor * r) {
	ax_link * node;
	ax_event * e;

	assert(r);

	if (ax_link_is_empty(&r->pending_list))
		return NULL;

	node = r->pending_list.next;
	e = ax_link_entry(node, ax_event, pending_link);

	ax_link_del(&e->pending_link);
	return e;
}

static ax_event * reactor_dequeue_event(ax_reactor * r) {
	ax_link * node;
	ax_event * e;

	assert(r != NULL);

	if (ax_link_is_empty(&r->event_list))return NULL;

	node = r->event_list.next;
	e = ax_link_entry(node, ax_event, event_link);

	ax_link_del(&e->event_link);
	return e;
}

int reactor_event_empty(ax_reactor * r) {
	assert(r != NULL);

	return ax_link_is_empty(&r->event_list);
}

void reactor_loop(ax_reactor * r, struct timeval * timeout, int flags) {
	int nreadys;
	struct timeval *pt, t;
	ax_event * e;

	assert(r != NULL);

	while (!r->out) {
		ax_mutex_lock(&r->lock);
		//ax_perror("start polling with timeout [%d, %d]", timeout ? timeout->tv_sec : -1, timeout ? timeout->tv_usec : -1);

		/*
		 * On linux, the select syscall modifies timeout
		 * to reflect the amount of time not slept.
		 * We have to reset the timeout in order to be portable.
		 */
		if (timeout == NULL) {
			pt = NULL;
		}else{
			t = *timeout;
			pt = &t;
		}

		if (r->pti) {
			/* 
			 * The timer event handling is supported,
			 * have @pt point to the smallest timeval.
			 */
			struct timeval t;
			struct timeval * timerv = timerheap_top_timeout(r, &t);
			if (timerv && (pt == NULL || (pt && timer_s(*timerv, *pt)))) {
				t = *timerv;
				pt = &t;
			}
		}
		nreadys = polling_poll(r, pt);
		//ax_perror("stopped polling, got %d readys", nreadys);
		if (r->pti) {
			/* handle timer events */
			reactor_handle_timer(r);
		}
		if (nreadys) {
			//iterate through pending events and call corresponding callbacks
			while (!r->out && (e = reactor_dequeue_pending(r))) {
				if (e->callback && event_in_reactor(e)) {
					ax_mutex_unlock(&r->lock);

					e->callback(e->fd, e->res_flags, e->callback_arg);

					ax_mutex_lock(&r->lock);
				}
			}
			if (flags & REACTOR_ONCE) {
				r->out = 1;
			}
		}
		ax_mutex_unlock(&r->lock);
	}
	//ax_perror("told to stop the loop, stopped.");
	//reset the flag for next loop
	r->out = 0;
}
