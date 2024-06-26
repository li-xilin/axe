/*
 * Copyright (c) 2022-2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to one person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "mux.h"
#include "event_ht.h"
#include "timer.h"
#include "reactor_type.h"

#include "ax/reactor.h"
#include "ax/event.h"
#include "ax/socket.h"
#include "ax/timeval.h"
#include "ax/log.h"
#include "ax/link.h"
#include "ax/reactor.h"

#ifdef AX_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/socket.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

static ax_event *reactor_dequeue_event(ax_reactor *r);

static void reactor_handle_timer(ax_reactor *r)
{
	while (timerheap_top_expired(r)) {
		ax_event *e = timerheap_get_top(r);
		assert(e);

		if (e->ev_flags & AX_EV_ONCE && e->timerheap_idx != E_OUT_OF_TIMERHEAP) {
			timerheap_remove_event(r, e);
			e->ev_flags &= ~AX_EV_REACTING;
		} else if (!(e->ev_flags & AX_EV_ONCE) && e->timerheap_idx != E_OUT_OF_TIMERHEAP) {
			timerheap_reset_timer(r, e);
		}

		ax_lock_put(&r->lock);
		e->cb(e->fd, e->res_flags, e->arg);
		ax_lock_get(&r->lock);
	}
}

static void reactor_waked_up(ax_socket fd, short res_flags, void *arg)
{
	char buf[1024];
	(void)recv(fd, buf, sizeof buf, 0);
}

static void reactor_wake_up(ax_reactor *r)
{
	char octet = 0;
	send(r->io_pipe[1], &octet, sizeof(octet), 0);
}

void ax_reactor_exit(ax_reactor *r)
{
	assert(r);

	ax_lock_get(&r->lock);
	r->out = 1;
	reactor_wake_up(r);
	ax_lock_put(&r->lock);
}

ax_reactor *ax_reactor_create()
{
	ax_reactor *r = malloc(sizeof *r);
	if (!r)
		goto fail;

	memset(r, 0, sizeof(ax_reactor));

	if (ax_lock_init(&r->lock))
		goto fail;

	r->eht = malloc(sizeof(struct ax_event_ht_st));
	if (!r->eht)
		goto fail;

	if (event_ht_init(r->eht, 0.5) == -1) {
		ax_perror("failed to initialize the hash table of events.");
		goto fail;
	}

	ax_link_init(&r->event_list);
	ax_link_init(&r->pending_list);

	r->mux = mux_init();
	if (!r->mux) {
		ax_perror("failed to initialize polling");
		goto fail;
	}

	r->out = 0;
	if (ax_socket_pair(AF_UNIX, SOCK_STREAM, 0, r->io_pipe) == -1) {
		ax_perror("failed to create informing pipe.");
		goto fail;
	}

	ax_event_set(&r->io_event, r->io_pipe[0], AX_EV_READ, reactor_waked_up, NULL);
	ax_reactor_add(r, &r->io_event);

	if ((r->pti = timerheap_init(r)) == NULL) {
		ax_perror("failed to initialize signal handling.");
		goto fail;
	}

	return r;
fail:
	if (r) {
		ax_lock_free(&r->lock);

		if (r->eht) {
			event_ht_free(r->eht);
			free(r->eht);
		}

		if (r->mux)
			mux_free(r->mux);

		if (r->io_pipe[0]) {
			ax_socket_close(r->io_pipe[0]);
			ax_socket_close(r->io_pipe[1]);
		}

		if (r->pti) {
			timerheap_destroy(r);
			free(r->pti);
			r->pti = NULL;
		}
	}

	return NULL;
}

static void reactor_free_events(ax_reactor *r)
{
	ax_event *e = NULL;

	while ((e = reactor_dequeue_event(r))) {
		mux_del(r->mux, e->fd, e->ev_flags);
		ax_link_del(&e->event_link);
		event_ht_delete(r->eht, e);
		ax_link_del(&e->pending_link);
		e->reactor = NULL;
	}
}

void ax_reactor_clear(ax_reactor *r)
{
	ax_lock_get(&r->lock);
	reactor_free_events(r);
	if (r->pti) {
		timerheap_clean_events(r);
	}
	ax_lock_put(&r->lock);
	/* remove all events except this informing pipe */
	ax_event_set(&r->io_event, r->io_pipe[0], AX_EV_READ, reactor_waked_up, NULL);
	if (ax_reactor_add(r, &r->io_event) == -1) {
		ax_perror("failed to add informing pipe event back to reactor");
	}
}

void ax_reactor_destroy(ax_reactor *r)
{
	if (!r)
		return;

	ax_lock_get(&r->lock);
	reactor_free_events(r);

	event_ht_free(r->eht);
	mux_free(r->mux);
	ax_socket_close(r->io_pipe[0]);
	ax_socket_close(r->io_pipe[1]);

	if (r->pti) {
		timerheap_destroy(r);
		free(r->pti);
	}
	ax_lock_put(&r->lock);
	ax_lock_free(&r->lock);
	free(r->eht);
	free(r);
}

int ax_reactor_add(ax_reactor *r, ax_event *e)
{
	assert(r != NULL && e != NULL);

	ax_lock_get(&r->lock);

	if (e->ev_flags & AX_EV_TIMEOUT) {
		int size = r->pti->size;
		ax_unused(size);
		assert(e->timerheap_idx == E_OUT_OF_TIMERHEAP);
		if (timerheap_add_event(r, e) == -1) {
			ax_lock_put(&r->lock);
			ax_perror("failed to register timer event");
			return -1;
		}
		assert(r->pti->size == size + 1);
		assert(e->timerheap_idx != E_OUT_OF_TIMERHEAP);
	}
	else {
		if (e->event_link.prev || e->event_link.next) {
			ax_lock_put(&r->lock);
			/*
			 *This event is already in the reactor.
			 *Assume every event only can be in one reactor.
			 */
			ax_perror("event already in the reactor");
			return -1;
		}
		if (mux_add(r->mux, e->fd, e->ev_flags) == -1) {
			ax_lock_put(&r->lock);
			ax_perror("failed to add the event[%d] to the reactor.", e->fd);
			return -1;
		}
		ax_link_add_back(&e->event_link, &r->event_list);

		event_ht_insert(r->eht, e, e->fd);
	}

	e->reactor = r;
	e->ev_flags |= AX_EV_REACTING;

	reactor_wake_up(r);

	ax_lock_put(&r->lock);

	return 0;
}

int ax_reactor_modify(ax_reactor *r, ax_event *e)
{
	assert(r != NULL && e != NULL);

	if (e->ev_flags & AX_EV_TIMEOUT) {
		timerheap_reset_timer(r, e);
		return 0;
	}

	ax_lock_get(&r->lock);

	if (mux_mod(r->mux, e->fd, e->ev_flags) == -1) {
		ax_lock_put(&r->lock);
		ax_perror("failed to modify the event[%d] in the reactor.", e->fd);
		return -1;
	}

	e->reactor = r;
	e->ev_flags |= AX_EV_REACTING;

	reactor_wake_up(r);

	ax_lock_put(&r->lock);
	return 0;
}

void ax_reactor_add_to_pending(ax_reactor *r, ax_event *e, short res_flags)
{
	assert(r != NULL && e != NULL);
	assert(ax_event_in_use(e));

	e->res_flags = res_flags;
	ax_link_add_back(&e->pending_link, &r->pending_list);
}

int ax_reactor_remove(ax_reactor *r, ax_event *e)
{
	assert(r != NULL && e != NULL);
	assert(ax_event_in_use(e));

	ax_lock_get(&r->lock);
	if (e->ev_flags & AX_EV_TIMEOUT) {
		if (e->timerheap_idx != E_OUT_OF_TIMERHEAP) {
			timerheap_remove_event(r, e);
		}
		assert(e->timerheap_idx == E_OUT_OF_TIMERHEAP);
	}
	else {
		mux_del(r->mux, e->fd, e->ev_flags);
		ax_link_del(&e->event_link);
		event_ht_delete(r->eht, e);
		ax_link_del(&e->pending_link);
	}

	e->reactor = NULL;
	e->ev_flags &= ~AX_EV_REACTING;

	reactor_wake_up(r);
	ax_lock_put(&r->lock);
	return 0;
}

static ax_event *reactor_dequeue_pending(ax_reactor *r)
{
	ax_link *node;
	ax_event *e;

	assert(r);

	if (ax_link_is_empty(&r->pending_list))
		return NULL;

	node = r->pending_list.next;
	e = ax_link_entry(node, ax_event, pending_link);

	ax_link_del(&e->pending_link);
	return e;
}

static ax_event *reactor_dequeue_event(ax_reactor *r)
{
	ax_link *node;
	ax_event *e;

	assert(r != NULL);

	if (ax_link_is_empty(&r->event_list))return NULL;

	node = r->event_list.next;
	e = ax_link_entry(node, ax_event, event_link);

	ax_link_del(&e->event_link);
	return e;
}

bool ax_reactor_empty(ax_reactor *r)
{
	assert(r != NULL);

	return ax_link_is_empty(&r->event_list);
}

static void pending_cb(ax_socket fd, short flags, void *arg)
{
	ax_reactor *r = arg;
	ax_event *e = event_ht_retrieve(r->eht, fd);
	ax_reactor_add_to_pending(r, e, flags);
}

void ax_reactor_loop(ax_reactor *r, struct timeval *timeout, int flags)
{
	assert(r != NULL);
	struct timeval *pt, t;

	while (!r->out) {
		ax_lock_get(&r->lock);
		/*
		 * On linux, the select syscall modifies timeout
		 * to reflect the amount of time not slept.
		 * We have to reset the timeout in order to be portable.
		 */
		if (!timeout) {
			pt = NULL;
		} else {
			t = *timeout;
			pt = &t;
		}

		if (r->pti) {
			/* 
			 *The timer event handling is supported,
			 *have @pt point to the smallest timeval.
			 */
			struct timeval t;
			struct timeval *timerv = timerheap_top_timeout(r, &t);
			if (timerv && (pt == NULL || (pt && ax_timeval_l(*timerv, *pt)))) {
				t = *timerv;
				pt = &t;
			}
		}

		int nreadys = mux_poll(r->mux, &r->lock, pt, pending_cb, r);
		reactor_handle_timer(r);

		if (nreadys) {
			//iterate through pending events and call corresponding callbacks
			ax_event *ev;
			while (!r->out && (ev = reactor_dequeue_pending(r))) {
				assert(ev->cb);
				ax_lock_put(&r->lock);
				ev->cb(ev->fd, ev->res_flags, ev->arg);
				ax_lock_get(&r->lock);
			}
			if (flags & AX_REACTOR_ONCE) {
				r->out = 1;
			}
		}
		ax_lock_put(&r->lock);
	}
	//reset the flag for next loop
	r->out = 0;
}

