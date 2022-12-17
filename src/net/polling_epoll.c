#include "event_ht.h"
#include "reactor_type.h"
#include "ax/reactor.h"
#include "ax/socket.h"
#include "ax/timeval.h"
#include "ax/log.h"

#ifdef AX_OS_WIN32
#include "wepoll.h"
#else
#include <sys/epoll.h>
#endif

#include <errno.h>
#include <unistd.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#define EPOLL_INIT_EVENT_SIZE 32

#ifdef AX_OS_WIN32
#define close_epoll(fd) CloseHandle(fd);
#define EPOLL_BAD_FD NULL
#else
#define close_epoll(fd) close(fd);
#define EPOLL_BAD_FD -1
#endif

struct epoll_internal
{
#ifdef AX_OS_WIN32
	HANDLE epoll_fd;
#else
	int epoll_fd;
#endif
	int n_events;
	int max_events;
	struct epoll_event * events;
};

static int epoll_resize(struct epoll_internal * pei, int size)
{
	struct epoll_event * pee;
	assert(pei != NULL);
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return -1;
	}

	if((pee = realloc(pei->events, size * sizeof(struct epoll_event))) == NULL) {
		ax_perror("failed to realloc for events, maybe run out of memory.");
		return -1;
	}

	pei->events = pee;
	pei->max_events = size;
	return 0;
}

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
	
	if((ret->epoll_fd = epoll_create(EPOLL_INIT_EVENT_SIZE)) == EPOLL_BAD_FD) {
		ax_perror("failed on epoll_create");
		free(ret);
		return NULL;
	}

	if(epoll_resize(ret, EPOLL_INIT_EVENT_SIZE) == -1) {
		ax_perror("failed on epoll_resize");
		close_epoll(ret->epoll_fd);
		free(ret);
		return NULL;
	}

	return ret;
}

static void epoll_free(struct epoll_internal * pei)
{
	assert(pei != NULL);
	free(pei->events);
	close_epoll(pei->epoll_fd);
	free(pei);
}

void polling_destroy(ax_reactor * r) {
	assert(r != NULL);
	epoll_free(r->polling_data);
}

static inline int epoll_setup_mask(short flags)
{
	int ret = 0;
	if(flags & AX_EV_READ) {
		ret |= EPOLLIN | EPOLLPRI;
	}
	if(flags & AX_EV_WRITE) {
		ret |= EPOLLOUT;
	}
	/*
	if(flags & AX_EV_EDGE) {
		ret |= EPOLLET;
	}
	*/
	if(flags & AX_EV_ONCE) {
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

int polling_add(ax_reactor * r, ax_socket fd, short flags)
{
	struct epoll_internal * pei;
	struct epoll_event e;
	int ret;
	
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return -1;
	}

	pei = r->polling_data;
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return -1;
	}

	if(pei->n_events >= pei->max_events) {
		ax_perror("resize to %d", pei->max_events << 1);
		if(epoll_resize(pei, pei->max_events << 1) == -1) {
			ax_perror("failed on epoll_resize");
			return -1;
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
		return -1;
	}
	success:
	//ax_perror("success on registering [fd %d] with this epoll instance", fd);
	++pei->n_events;
	return 0;
}

int polling_mod(ax_reactor * r, ax_socket fd, short flags)
{
	struct epoll_internal * pei;
	struct epoll_event e;
	int ret;
	
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return -1;
	}

	pei = r->polling_data;
	if(pei == NULL) {
		ax_perror("pei is null!!");
		return -1;
	}

	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	ret = epoll_ctl(pei->epoll_fd, EPOLL_CTL_MOD, fd, &e);

	/* Error handling*/
	if(ret) {
		epoll_print_error(pei, fd);
		return -1;
	}

	return 0;
}

void polling_del(ax_reactor * r, ax_socket fd, short flags)
{
	struct epoll_internal * pei;
	struct epoll_event e;
	assert(r != NULL);

	pei = r->polling_data;

	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	(void)epoll_ctl(pei->epoll_fd, EPOLL_CTL_DEL, fd, &e);

	--pei->n_events;
}

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
			res_flags |= AX_EV_READ;
		}
		if(pei->events[i].events & EPOLLOUT) {
			res_flags |= AX_EV_WRITE;
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
				ax_reactor_add_to_pending(r, e, res_flags);
			}
		}
	}

	return nreadys;
}

void polling_print(ax_reactor * r)
{
	ax_perror("empty implementation of epoll_print.");
}
