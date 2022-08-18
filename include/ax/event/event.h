#ifndef AX_EVENT_EVENT_H
#define AX_EVENT_EVENT_H

#include "util.h"
#include "../link.h"

#define E_READ  	0x1
#define E_WRITE 	0x2
#define E_SIGNAL	0x4 
#define E_TIMEOUT	0x8 
#define E_EDGE 		0x10/* edge triggered */
#define E_ONCE		0x20/* one-time event */
#define E_IN_REACTOR	0x40

/* 
* The timer event's initial timerheap_idx value,
* indicates that the event is not in the heap.
*/
#define E_OUT_OF_TIMERHEAP 0 

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

#ifndef AX_EVENT_DEFINED
#define AX_EVENT_DEFINED
typedef struct ax_event_st ax_event;
#endif

typedef void (*event_callback)(ax_socket fd, short res_flags, void *arg);

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
	ax_reactor * rc;
	
	event_callback callback;
	void * callback_arg;

	/* Only used for timer event */
	int timerheap_idx;
};

/*
* Initialize a given event.
* For I/O events, fd stores the file descriptor being listened.
* For signal events, fd stores the signal number being listened.
* For timer events, fd stores the timer interval in millisecond.
*/
void ax_event_set(ax_event *e, ax_socket fd, short ev_flags,event_callback callback, void *arg);

bool ax_event_in_use(const ax_event *e);

#endif
