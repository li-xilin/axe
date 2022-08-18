/*
* Copyright (c) 2014 Xinjing Chow
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERR
*/
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ax/event/event.h"
#include "ax/event/skutil.h"
#include "ax/link.h"

/*
* Create and initialize a new event.
* Return: newly created event.
* @fd: the file descriptor related.
* @ev_flags: the events interested
* @callback: the function to call when specific event has occrued
* @arg: the arg which pass to the @callback, if null we will pass the event to the callback.
*/
inline ax_event * event_new(ax_socket fd, short ev_flags,event_callback callback, void * arg){
	if(callback == NULL)return NULL;
	ax_event * new_event = malloc(sizeof(ax_event));
	
	if(new_event == NULL)return NULL;
	new_event->ev_flags = ev_flags;
	new_event->callback = callback;
	new_event->callback_arg = arg ? arg : new_event;
	new_event->rc = NULL;
	new_event->fd = fd;
	new_event->timerheap_idx = E_OUT_OF_TIMERHEAP;
	ax_link_init_empty(&new_event->event_link);
	ax_link_init_empty(&new_event->pending_link);
	ax_link_init_empty(&new_event->hash_link);
	return new_event;
}

inline void event_set(ax_event * e, ax_socket fd, short ev_flags, event_callback callback, void * arg){
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

void event_modify_events(ax_event * e, short ev_flags) {
	assert(e != NULL);

	e->ev_flags = ev_flags;
}

inline int event_in_reactor(ax_event * e){
	return e->ev_flags & E_IN_REACTOR;
}
