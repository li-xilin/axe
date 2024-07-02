/*
 * Copyright (c) 2022-2023 Li xilin <lixilin@gmx.com>
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

#ifndef AX_EVENT_H
#define AX_EVENT_H

#include "socket.h"
#include "link.h"

#define AX_EV_READ        (1 << 0)
#define AX_EV_WRITE       (1 << 1)
#define AX_EV_TIMEOUT     (1 << 2)
#define AX_EV_ERROR       (1 << 3)
#define AX_EV_ONCE        (1 << 4)
#define AX_EV_REACTING    (1 << 5)

struct ax_reactor_st;

#ifndef AX_EVENT_DEFINED
#define AX_EVENT_DEFINED
typedef struct ax_event_st ax_event;
#endif

typedef void (ax_event_cb_f)(ax_socket fd, short res_flags, void *arg);

struct ax_event_st
{
	ax_link event_link;
	ax_link pending_link;
	ax_link hash_link;
	
	ax_socket fd;

	/* Event type bitmask */
	short ev_flags;
	/* The events have been set after polling */
	short res_flags;

	/* Back pointer of the reactor holds the event */
	struct ax_reactor_st *reactor;
	
	ax_event_cb_f *cb;
	void *arg;

	/* Only used for timer event */
	int timerheap_idx;
};

/*
 * Initialize a given event.
 * For I/O events, fd stores the file descriptor being listened.
 * For timer events, fd stores the timer interval in millisecond.
 */
void ax_event_set(ax_event *e, ax_socket fd, short ev_flags,ax_event_cb_f *cb, void *arg);

inline static ax_event ax_event_make(ax_socket fd, short ev_flags,ax_event_cb_f *cb, void *arg)
{
	ax_event event;
	ax_event_set(&event, fd, ev_flags, cb, arg);
	return event;
}

inline static ax_event ax_event_for_socket(ax_socket fd, short ev_flags, ax_event_cb_f *cb, void *arg)
{
	return ax_event_make(fd, ev_flags, cb, arg);
}

inline static ax_event ax_event_for_timeout(unsigned millisec, bool once, ax_event_cb_f *cb, void *arg)
{
	return ax_event_make(millisec, AX_EV_TIMEOUT | (once ? AX_EV_ONCE : 0), cb, arg);
}

bool ax_event_in_use(const ax_event *e);

#endif
