#include "../mux.h"
#include "../event_ht.h"
#include "ax/reactor.h"
#include "ax/socket.h"
#include "ax/timeval.h"
#include "ax/log.h"

#ifdef AX_OS_WIN32
#include "../wepoll.h"
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

struct mux_st
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

static int epoll_resize(struct mux_st * mux, int size)
{
	struct epoll_event * pee;
	assert(mux != NULL);
	if (mux == NULL) {
		ax_perror("mux is null!!");
		return -1;
	}

	if ((pee = realloc(mux->events, size * sizeof(struct epoll_event))) == NULL) {
		ax_perror("failed to realloc for events, maybe run out of memory.");
		return -1;
	}

	mux->events = pee;
	mux->max_events = size;
	return 0;
}

mux *mux_init(void)
{
	mux* ret = NULL;;

	if ((ret = malloc(sizeof(struct mux_st))) == NULL) {
		ax_perror("failed to malloc for mux_st");
		return NULL;
	}

	memset(ret, 0, sizeof(struct mux_st));
	
	if ((ret->epoll_fd = epoll_create(EPOLL_INIT_EVENT_SIZE)) == EPOLL_BAD_FD) {
		ax_perror("failed on epoll_create");
		free(ret);
		return NULL;
	}

	if (epoll_resize(ret, EPOLL_INIT_EVENT_SIZE) == -1) {
		ax_perror("failed on epoll_resize");
		close_epoll(ret->epoll_fd);
		free(ret);
		return NULL;
	}

	return ret;
}

void mux_free(mux * mux)
{
	if (!mux)
		return;
	free(mux->events);
	close_epoll(mux->epoll_fd);
	free(mux);
}


static inline int epoll_setup_mask(short flags)
{
	int ret = 0;
	if (flags & AX_EV_READ)
		ret |= EPOLLIN | EPOLLPRI;
	if (flags & AX_EV_WRITE)
		ret |= EPOLLOUT;
	/*
	if (flags & AX_EV_EDGE) {
		ret |= EPOLLET;
	}
	*/
	if (flags & AX_EV_ONCE)
		ret |= EPOLLONESHOT;
	return ret;
}

static void epoll_print_error(struct mux_st * mux, ax_socket fd)
{
	if (errno == EBADF) {
		ax_perror("[epoll_fd %d]or [fd %d]is not valid!!", mux->epoll_fd, fd);
	} else if (errno == ENOENT) {
	         ax_perror("[fd %d] is not registered with this epoll instance.", fd);
	} else if (errno == EINVAL) {
	         ax_perror("[epoll_fd %d] is not an epoll file descriptor, or [fd %d]is the same as [epoll_fd %d],or the requested operation EPOLL_CTL_ADD is not supported  by  this interface.", mux->epoll_fd, fd, mux->epoll_fd);
	} else if (errno == ENOMEM) {
	         ax_perror("memory shorage");
	} else if (errno == ENOSPC) {
	         ax_perror("The limit imposed by /proc/sys/fs/epoll/max_user_watches exceeded.");
	} else if (errno == EPERM) {
		ax_perror("The target file [fd %d] does not support epoll. "
			"It's meaningless to epolling on regular files, read this post through [ http://www.groupsrv.com/linux/about159067.html ].", fd);
	}
}

int mux_add(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	struct epoll_event e;
	int ret;
	
	if (mux->n_events >= mux->max_events) {
		ax_perror("resize to %d", mux->max_events << 1);
		if (epoll_resize(mux, mux->max_events << 1) == -1) {
			ax_perror("failed on epoll_resize");
			return -1;
		}
	}
	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	ret = epoll_ctl(mux->epoll_fd, EPOLL_CTL_ADD, fd, &e);

	/* Error handling*/
	if (ret) {
		if (errno == EEXIST) {
			ax_perror("[fd %d]is alredy registered with this epoll instance, retry with EPOLL_CTL_MOD.", fd);
			/* retry with EPOLL_CTL_MOD */
			ret = epoll_ctl(mux->epoll_fd, EPOLL_CTL_MOD, fd, &e);

			if (ret == 0) 
				goto success;
			epoll_print_error(mux, fd);
		}else{
			epoll_print_error(mux, fd);
		}
		return -1;
	}
success:
	++mux->n_events;
	return 0;
}

int mux_mod(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	struct epoll_event e;
	int ret;
	
	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	ret = epoll_ctl(mux->epoll_fd, EPOLL_CTL_MOD, fd, &e);

	/* Error handling*/
	if (ret) {
		epoll_print_error(mux, fd);
		return -1;
	}

	return 0;
}

void mux_del(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	struct epoll_event e;

	e.data.fd = fd;
	e.events = epoll_setup_mask(flags);
	
	(void)epoll_ctl(mux->epoll_fd, EPOLL_CTL_DEL, fd, &e);

	--mux->n_events;
}

int mux_poll(mux *mux, ax_lock *lock, struct timeval * timeout, mux_pending_cb *pending_cb, void *arg)
{
	int res_flags , nreadys, i;

	assert(mux != NULL);

	ax_lock_put(lock);
	nreadys = epoll_wait(mux->epoll_fd, mux->events, mux->n_events,
			timeout ? timeout->tv_sec * 1000 + timeout->tv_usec / 1000 : -1 );
	ax_lock_get(lock);

	for(i = 0; i < nreadys; ++i) {
		res_flags = 0;
		if (mux->events[i].events & (EPOLLIN | EPOLLPRI))
			res_flags |= AX_EV_READ;
		if (mux->events[i].events & EPOLLOUT)
			res_flags |= AX_EV_WRITE;
		if (mux->events[i].events & EPOLLERR)
			res_flags |= AX_EV_ERROR;
		if (res_flags)
			pending_cb(mux->events[i].data.fd, res_flags, arg);
	}

	return nreadys;
}

