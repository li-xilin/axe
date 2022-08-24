#ifndef AX_EVENT_EVENT_H
#define AX_EVENT_EVENT_H

#include "socket.h"
#include "../link.h"

#define AX_EV_READ        (1 << 0)
#define AX_EV_WRITE       (1 << 1)
#define AX_EV_TIMEOUT     (1 << 2)
#define AX_EV_EDGE        (1 << 3)
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
* For signal events, fd stores the signal number being listened.
* For timer events, fd stores the timer interval in millisecond.
*/
void ax_event_set(ax_event *e, ax_socket fd, short ev_flags,ax_event_cb_f *cb, void *arg);

bool ax_event_in_use(const ax_event *e);

#endif
