#ifndef AX_EVENT_TIMER_H
#define AX_EVENT_TIMER_H

#include <sys/time.h>
#include "ax/event/util.h"

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

#ifndef AX_EVENT_DEFINED
#define AX_EVENT_DEFINED
typedef struct ax_event_st ax_event;
#endif

struct heap_entry; //TODO namespace

/*
struct heap_entry {
	struct timeval expiration;
	ax_event * e;
};
*/

struct timerheap_st {
	/* The number of entries in the heap */
	int size;
	/* The capacity of the heap */
	int capacity;
	/* The heap_entry array */
	struct heap_entry * heap;
};

/*
* Add v milliseconds to the timeval t
*/
#define timer_add(t, v)do{				\
	(t).tv_sec += (v) / 1000;			\
	(t).tv_usec += ((v) % 1000) * 1000;	\
	if((t).tv_usec > 1000000){			\
		(t).tv_usec -= 1000000;			\
		(t).tv_sec++;					\
	}									\
}while(0);

/*
* Substract v milliseconds from the timeval t
*/
#define timer_sub(t, v)do{				\
	(t).tv_sec -= (v) / 1000;			\
	(t).tv_usec -= ((v) % 1000) * 1000;	\
	if((t).tv_usec < 0){				\
		(t).tv_usec += 1000000;			\
		(t).tv_sec--;					\
	}									\
}while(0);

/*
* Convert the time in millisecond to timerval.
*/
#define timer_to_tv(tv, tm)do{			\
	(tv).tv_sec = (tm) / 1000;			\
	(tv).tv_usec = ((tm) % 1000) * 1000;	\
}while(0);								
/*
* Convert the timerval to time in millisecond.
*/
#define timer_to_ms(t) ((t).tv_sec * 1000 + (t).tv_usec / 1000)

/*
* Tests whether @t1 is greater or equal to @t2.
*/
#define timer_ge(t1, t2) ((t1).tv_sec > (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec >= (t2).tv_usec))

/*
* Tests whether @t1 is smaller or equal to @t2.
*/
#define timer_se(t1, t2) ((t1).tv_sec < (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec <= (t2).tv_usec))

/*
* Tests whether @t1 is greater to @t2.
*/
#define timer_g(t1, t2) ((t1).tv_sec > (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec > (t2).tv_usec))

/*
* Tests whether @t1 is smaller to @t2.
*/
#define timer_s(t1, t2)  ((t1).tv_sec < (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec < (t2).tv_usec))


#define TIMERHEAP_INIT_SIZE 32

/*
* Create and initialize a timerheap.
* Return: a newly created timerheap_internal structure on success, NULL on failure.
*/
struct timerheap_st * timerheap_init();

/*
* Tests whether the top entry is expired.
* Return: 0 on false, 1 on true.
* @r: the reactor which handles the timer events.
*/
int timerheap_top_expired(ax_reactor * r);
/*
* Get the top entry's timeout value.
* Return: the expiration stored in timeval on success, NULL on failure(The timerheap is empty).
* @r: the reactor which handles the timer events.
* @timeout: the timeval struct to hold the return val
*/
struct timeval * timerheap_top_timeout(ax_reactor * r, struct timeval * timeout);
/*
* Retrieve the top event of the timerheap.
* Return: the top entry of the heap on success, NULL if the heap is empty.
* @r: the reactor which handles the timer events.
*/
ax_event * timerheap_get_top(ax_reactor * r);

/*
* Pop up the top event of the timerheap and reheapify the timerheap.
* Return: the top entry of the heap on success, NULL if the heap is empty.
* @r: the reactor which handles the timer events.
*/
ax_event * timerheap_pop_top(ax_reactor * r);

/*
* Add the timer event by its interval and reajust the heap.
* @r: the reactor which handles the timer events.
* @e: the timer event being manipulated.
*/
void timerheap_reset_timer(ax_reactor * r, ax_event * e);

/*
* Add timer event to the timerheap.
* Return: 0 on success, -1 on failure.
* @r: the reactor which handles the timer events.
* @e: the timer event to add.
*/
int timerheap_add_event(ax_reactor * r, ax_event * e);

/*
* Remove timer event from the timerheap.
* Return: 0 on success, -1 on failure.
* @r: the reactor which handles the timer events.
* @e: the timer event to remove.
*/
void timerheap_remove_event(ax_reactor * r, ax_event * e);

/*
* remove all timer events from the timerheap.
* Return: 0 on success, -1 on failure.
* @r: the reactor which handles the timer events.
*/
void timerheap_clean_events(ax_reactor *r);

/*
* Free up the resources used by the timerheap and the timerheap_internal structure.
* Return: 0 on success, -1 on failure.
* @r: the reactor which handles the timer events.
*/
void timerheap_destroy(ax_reactor * r);

#endif
