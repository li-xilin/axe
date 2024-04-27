#include "../mux.h"

#include "ax/timeval.h"
#include "ax/log.h"
#include <sys/errno.h>
#include <poll.h>
#include <memory.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

#define POLL_INIT_EVENT_SIZE 32

struct mux_st
{
	/* the number of pollfds that fds_in pointed to */
	int max_events;
	/* the number of pollfds that we have added to fds_in */
	int n_events;
	/* 
	 * Indicates whether we need to realloc the fds to match 
	 * max_events, since we might have changed the
	 * size of memory that fds_in pointed to.
	 */
	int need_realoc;
	/*
	 * Indicates whether we need memcpy from fds_in to fds.
	 */
	int need_rememcp;
	struct pollfd *fds;
	/* 
	 * In multithreaded environment, we may encounter situation that
	 * main thread is polling while another thread trying to modify the fds.
	 * So we introduce the field to avoid this kind of race condition.
	 * If we are in singlethreaded environment, we simply point fds to fds_in.
	 */
	struct pollfd *fds_in;
};

static int poll_resize(struct mux_st * mux, int size)
{
	struct pollfd * pfds;
	assert(mux != NULL);
	if(mux == NULL) {
		ax_perror("mux is null!!");
		return -1;
	}

	if((pfds = realloc(mux->fds_in, size * sizeof(struct pollfd))) == NULL) {
		ax_perror("failed to realloc for fds, maybe run out of memory.");
		return -1;
	}

	mux->fds_in = pfds;
	mux->max_events = size;
	mux->need_realoc = 1;
	mux->need_rememcp = 1;
	return 0;
}

mux *mux_init(void)
{
	struct mux_st * ret;

	if((ret = malloc(sizeof(struct mux_st))) == NULL) {
		ax_perror("failed to malloc for mux_st");
		return NULL;
	}

	memset(ret, 0, sizeof(struct mux_st));

	if(poll_resize(ret, POLL_INIT_EVENT_SIZE) == -1) {
		ax_perror("failed on poll_resize");
		free(ret);
		return NULL;
	}

	return ret;
}

void mul_free(mux *mux)
{
	if (!mux)
		return;

	if(mux->fds)
		free(mux->fds);
	if(mux->fds_in)
		free(mux->fds_in);

	free(mux);
}

static inline int poll_setup_mask(short flags)
{
	int ret = 0;
	if(flags & AX_EV_READ) {
		ret |= POLLIN | POLLPRI;
	}
	if(flags & AX_EV_WRITE) {
		ret |= POLLOUT;
	}
	return ret;
}

int mux_add(mux *mux, ax_socket fd, short flags)
{
	assert(mux);

	int i;

	for(i = 0; i < mux->n_events; ++i) {
		if(fd == mux->fds_in[i].fd) {
			/* Override the existing event */
			mux->fds_in[i].events = poll_setup_mask(flags);
			return 0;
		}
	}
	if(mux->n_events >= mux->max_events) {
		ax_perror("resize to %d", mux->max_events << 1);
		if(poll_resize(mux, mux->max_events << 1) == -1) {
			ax_perror("failed on poll_resize");
			return -1;
		}
	}
	mux->fds_in[i].fd = fd;
	mux->fds_in[i].events = poll_setup_mask(flags);
	mux->need_rememcp = 1;;
	++mux->n_events;

	return 0;
}

int mux_mod(mux *mux, ax_socket fd, short flags)
{
	assert(mux);
	int i;

	for(i = 0; i < mux->n_events; ++i) {
		if(fd == mux->fds_in[i].fd) {
			/* Override the existing event */
			mux->fds_in[i].events = poll_setup_mask(flags);
			return 0;
		}
	}

	ax_perror("fd[%d] is not in the poll fdset.", fd);
	return -1;
}

static inline void poll_swap_pollfd(struct pollfd * lhs, struct pollfd * rhs)
{
	struct pollfd t;
	t = *lhs;
	*lhs = *rhs;
	*rhs = t;
}

void mux_del(mux *mux, ax_socket fd, short flags)
{
	assert(mux);

	for(int i = 0; i < mux->n_events; ++i) {
		if(mux->fds_in[i].fd == fd) {
			poll_swap_pollfd(&mux->fds_in[i], &mux->fds_in[mux->n_events - 1]);
			mux->need_rememcp = 1;
			mux->n_events --;
			break;
		}
	}
}

int mux_poll(mux *mux, ax_lock *lock, struct timeval * timeout, mux_pending_cb *pending_cb, void *arg)
{
	assert(mux);

	int res_flags, nreadys;

	if(mux->need_realoc) {
		if((mux->fds = realloc(mux->fds, mux->max_events * sizeof(struct pollfd))) == NULL) {
			ax_perror("failed to realloc for mux->fds");
			exit(1);
		}
		mux->need_realoc = 0;
	}
	if(mux->need_rememcp) {
		memcpy(mux->fds, mux->fds_in, mux->max_events * sizeof(struct pollfd));
		mux->need_rememcp = 0;
	}
	/* No need to realloc and rememcpy since we are in single-threaeded environment. */
	/* mux->fds = mux->fds_in; */

	ax_lock_put(lock);
	nreadys = poll(mux->fds, 
			mux->n_events, 
			timeout ? timeout->tv_sec * 1000 + timeout->tv_usec / 1000 : -1);
	ax_lock_get(lock);

	if(nreadys) {
		for(int i = 0; i < mux->n_events; ++i) {
			res_flags = 0;
			if (mux->fds[i].revents & (POLLIN | POLLPRI))
				res_flags |= AX_EV_READ;
			if (mux->fds[i].revents & POLLOUT)
				res_flags |= AX_EV_WRITE;
			if (mux->fds[i].revents & POLLERR)
				res_flags |= AX_EV_ERROR;

			if (mux->fds[i].revents & POLLNVAL)
				ax_perror("[fd %d] is invalid!", mux->fds[i].fd);

			if (res_flags)
				pending_cb(mux->fds[i].fd, res_flags, arg);
		}
	}
	return nreadys;
}

