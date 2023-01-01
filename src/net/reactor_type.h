#ifndef EVENT_REACTOR_TYPE_H
#define EVENT_REACTOR_TYPE_H

#include "ax/link.h"
#include "ax/socket.h"
#include "ax/event.h"
#include "ax/mutex.h"

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

struct timerheap_st;
struct ax_event_ht_st;

/* 
* The timer event's initial timerheap_idx value,
* indicates that the event is not in the heap.
*/
#define E_OUT_OF_TIMERHEAP 0 

struct ax_reactor_st
{
	ax_link event_list;

	ax_link pending_list;

	void *polling_data;

	struct ax_event_ht_st *eht;

	ax_mutex lock;

	ax_socket io_pipe[2];

	ax_event io_event;

	struct timerheap_st *pti;

	ax_socket timer_pipe[2];

	ax_event *timer_event;

	/* Indicates whether we should stop polling. */
	int out;
};

#endif
