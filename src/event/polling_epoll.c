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
/* epoll polling policy */
#include "event_ht.h"
#include "ax/event/reactor.h"
#include "ax/event/skutil.h"
#include "ax/log.h"

#include <sys/errno.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#define EPOLL_INIT_EVENT_SIZE 32

struct epoll_internal
{
	int epoll_fd;
	int n_events;
	int max_events;
	struct epoll_event * events;
};

/*
* Resize the events to given size.
* Return: -1 on failure, 0 on success.
* @pei: the internal data used by epoll polling policy.
* @size: the size that we should resize to.
*/
static int epoll_resize(struct epoll_internal * pei, int size)
{
	struct epoll_event * pee;
	assert(pei != NULL);
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return (-1);
	}

	if((pee = realloc(pei->events, size * sizeof(struct epoll_event))) == NULL) {
		ax_perror("failed to realloc for events, maybe run out of memory.");
		return (-1);
	}

	pei->events = pee;
	pei->max_events = size;
	return (0);
}

/*
* Create and initialize the internal data used by epoll polling policy.
* Return value: newly created internal data on success, NULL on failure.
* @r: the reactor which uses this policy.
*/
void * polling_init(ax_reactor * r)
{
	struct epoll_internal * ret;

	assert(r);
	if(r == NULL) {
		ax_perror("r is null!!");
		return NULL;
	}
	
	if((ret = malloc(sizeof(struct epoll_internal))) == NULL) {
		ax_perror("failed to malloc for epoll_internal");
		return NULL;
	}

	memset(ret, 0, sizeof(struct epoll_internal));
	
	if((ret->epoll_fd = epoll_create(EPOLL_INIT_EVENT_SIZE)) == -1) {
		ax_perror("failed on epoll_create");
		free(ret);
		return NULL;
	}

	if(epoll_resize(ret, EPOLL_INIT_EVENT_SIZE) == -1) {
		ax_perror("failed on epoll_resize");
		close(ret->epoll_fd);
		free(ret);
		return NULL;
	}

	return ret;
}

/* 
* Frees up the internal data used by epoll polling policy.
* @pei: the internal data.
*/
static void epoll_free(struct epoll_internal * pei)
{
	assert(pei != NULL);
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return;
	}

	if(pei->events) {
		free(pei->events);
		pei->events = NULL;
	}
	if(pei->epoll_fd >= 0) {
		close(pei->epoll_fd);
	}

	free(pei);
}

/*
* Clean up the policy internal data
* @r: the reactor which uses this policy
*/
void polling_destroy(ax_reactor * r) {
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return;
	}
	epoll_free(r->polling_data);
}

static inline int epoll_setup_mask(short flags)
{
	int ret = 0;
	if(flags & E_READ) {
		ret |= EPOLLIN | EPOLLPRI;
	}
	if(flags & E_WRITE) {
		ret |= EPOLLOUT;
	}
	if(flags & E_EDGE) {
		ret |= EPOLLET;
	}
	if(flags & E_ONCE) {
		ret |= EPOLLONESHOT;
	}
	return ret;
}

static void epoll_print_error(struct epoll_internal * pei, ax_socket fd)
{
	if(errno == EBADF) {
		ax_perror("[epoll_fd %d]or [fd %d]is not valid!!", pei->epoll_fd, fd);
	}else if(errno == ENOENT) {
		ax_perror("[fd %d] is not registered with this epoll instance.", fd);
	}else if(errno == EINVAL) {
		ax_perror("[epoll_fd %d] is not an epoll file descriptor, or [fd %d]is the same as [epoll_fd %d],or the requested operation EPOLL_CTL_ADD is not supported  by  this interface.", pei->epoll_fd, fd, pei->epoll_fd);
	}else if(errno == ENOMEM) {
		ax_perror("memory shorage");
	}else if(errno == ENOSPC) {
		ax_perror("The limit imposed by /proc/sys/fs/epoll/max_user_watches exceeded.");
	}else if(errno == EPERM) {
		ax_perror("The target file [fd %d] does not support epoll. "
			"It's meaningless to epolling on regular files, read this post through [ http://www.groupsrv.com/linux/about159067.html ].", fd);
	}
}

/*
* Register the given file descriptor with this epoll instance.
* Return: 0 on success, -1 on failure.
* @r: the reactor which uses this policy.
* @fd: the file descriptor to listen.
* @flags: the interested events.
*/
int polling_add(ax_reactor * r, ax_socket fd, short flags)
{
	struct epoll_internal * pei;
	struct epoll_event e;
	int ret;
	
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return (-1);
	}

	pei = r->polling_data;
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return (-1);
	}

	if(pei->n_events >= pei->max_events) {
		ax_perror("resize to %d", pei->max_events << 1);
		if(epoll_resize(pei, pei->max_events << 1) == -1) {
			ax_perror("failed on epoll_resize");
			return (-1);
		}
	}
	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	ret = epoll_ctl(pei->epoll_fd, EPOLL_CTL_ADD, fd, &e);

	/* Error handling*/
	if(ret) {
		if(errno == EEXIST) {
			ax_perror("[fd %d]is alredy registered with this epoll instance, retry with EPOLL_CTL_MOD.", fd);
			/* retry with EPOLL_CTL_MOD */
			ret = epoll_ctl(pei->epoll_fd, EPOLL_CTL_MOD, fd, &e);

			if(ret == 0) 
				goto success;
			epoll_print_error(pei, fd);
		}else{
			epoll_print_error(pei, fd);
		}
		return (-1);
	}
	success:
	//ax_perror("success on registering [fd %d] with this epoll instance", fd);
	++pei->n_events;
	return (0);
}


/*
* Modify the interested events of a fd.
* Return: 0 on success, -1 on failure.
* @r: the reactor which uses this policy.
* @fd: the file descriptor to listen.
* @flags: the interested events.
*/
int polling_mod(ax_reactor * r, ax_socket fd, short flags)
{
	struct epoll_internal * pei;
	struct epoll_event e;
	int ret;
	
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return (-1);
	}

	pei = r->polling_data;
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return (-1);
	}

	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	ret = epoll_ctl(pei->epoll_fd, EPOLL_CTL_MOD, fd, &e);

	/* Error handling*/
	if(ret) {
		epoll_print_error(pei, fd);
		return (-1);
	}

	return (0);
}

/*
* Unregister the given file descriptor with this epoll instance.
* Return: -1 on failure, 0 on success.
* @r: the reactor which uses this policy.
* @fd: the file descriptor to remove.
* @flags: the interested events.
*/
int polling_del(ax_reactor * r, ax_socket fd, short flags)
{
	struct epoll_internal * pei;
	struct epoll_event e;
	int ret;
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return (-1);
	}

	pei = r->polling_data;
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return (-1);
	}

	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	ret = epoll_ctl(pei->epoll_fd, EPOLL_CTL_DEL, fd, &e);

	if(ret) {
		epoll_print_error(pei, fd);
		return (-1);
	}

	//ax_perror("success on unregistering [fd %d] with this epoll instance", fd);
	--pei->n_events;
	return (0);
}

/*
* Polling the file descriptor via epoll and add active events to the pending_list of the reactor.
* @r: the reactor which uses this policy.
* @timeout: the time after which the select will return.
*/
int polling_poll(ax_reactor * r, struct timeval * timeout)
{
	int res_flags , nreadys, i;
	struct epoll_internal * pei;
	ax_event * e;

	assert(r != NULL);

	pei = r->polling_data;
	
	assert(pei != NULL);

	ax_mutex_unlock(&r->lock);
	nreadys = epoll_wait(pei->epoll_fd, pei->events, pei->n_events,
			timeout ? timeout->tv_sec * 1000 + timeout->tv_usec / 1000 : -1 );
	ax_mutex_lock(&r->lock);

	for(i = 0; i < nreadys; ++i) {
		res_flags = 0;
		if(pei->events[i].events & (EPOLLIN | EPOLLPRI)) {
			res_flags |= E_READ;
		}
		if(pei->events[i].events & EPOLLOUT) {
			res_flags |= E_WRITE;
		}
		if(pei->events[i].events & EPOLLERR) {
			ax_perror("got a EPOLLERR event: %s", strerror(errno));
		}
		if(res_flags) {
			e = event_ht_retrieve(r->eht, pei->events[i].data.fd);
				
			assert(e != NULL);
			if(e == NULL) {
				ax_perror("the event with [fd %d] is not in the hashtable", pei->events[i].data.fd);
			}else{
				reactor_add_to_pending(r, e, res_flags);
			}
		}
	}

	return nreadys;
}
/* Dumps out the internal data of select policy for debugging. */
void polling_print(ax_reactor * r)
{
	ax_perror("empty implementation of epoll_print.");
}
