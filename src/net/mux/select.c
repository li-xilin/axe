#include "../mux.h"

#include "ax/socket.h"
#include "ax/event.h"
#include "ax/hmap.h"
#include "ax/iter.h"

#ifdef AX_OS_WIN32
#include <winsock.h>
#include <windef.h>
#else
#include <sys/select.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

struct mux_st
{
	int n_readfd, n_writefd;
	ax_hmap_r fds;
};

mux *mux_init(void)
{
	struct mux_st *mux = NULL;

	mux = malloc(sizeof *mux);
	if (!mux)
		goto fail;

	mux->fds.ax_one = NULL;
	mux->n_readfd = mux->n_writefd = 0;

	mux->fds = ax_new(ax_hmap, ax_t(ax_socket), ax_t(short));
	if (ax_r_isnull(mux->fds))
		goto fail;
	return mux;
fail:
	if (mux) {
		ax_one_free(mux->fds.ax_one);
		free(mux);
	}
	return NULL;
}

int mux_add(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	if (mux->n_readfd + (flags & AX_EV_READ) == FD_SETSIZE
			|| mux->n_writefd + (flags & AX_EV_WRITE) == FD_SETSIZE) {
		ax_perror("Number of fd exceeds the limit");
		return -1;
	}

	if (flags & AX_EV_READ)
		mux->n_readfd ++;
	if (flags & AX_EV_WRITE)
		mux->n_writefd ++;

	short flags_insert = flags & (AX_EV_READ|AX_EV_WRITE);
	if (ax_map_exist(mux->fds.ax_map, &fd))
		return -1;
	if (!ax_map_put(mux->fds.ax_map, &fd, &flags_insert))
		return -1;

	return 0;
}

int polling_mod(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	ax_iter it = ax_map_at(mux->fds.ax_map, &fd);
	if (ax_box_iter_ended(mux->fds.ax_box, &it))
		return -1;

	short *pflags = ax_iter_get(&it);

	int read_changed = ((*pflags ^ flags)) & AX_EV_READ ? (!!(flags & AX_EV_READ) << 1) - 1 : 0;
	int write_changed = ((*pflags ^ flags)) & AX_EV_WRITE ? (!!(flags & AX_EV_WRITE) << 1) - 1 : 0;

	if (mux->n_readfd + read_changed == FD_SETSIZE
			|| mux->n_writefd + write_changed == FD_SETSIZE) {
		ax_perror("Number of fd exceeds the limit");
		return -1;
	}
	mux->n_readfd += read_changed;
	mux->n_writefd += write_changed;

	*pflags = flags;
	return 0;
}

void mux_del(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	short old_flags= *(short *)ax_map_get(mux->fds.ax_map, &fd);

	if ((old_flags & AX_EV_READ) && (flags & AX_EV_READ))
		mux->n_readfd--;
	if ((old_flags & AX_EV_WRITE) && (flags & AX_EV_WRITE))
		mux->n_readfd--;

	short new_flags = old_flags = old_flags & (~flags);
	if (!(new_flags & (AX_EV_READ|AX_EV_WRITE)))
		ax_map_erase(mux->fds.ax_map, &fd);
}

int mux_poll(mux *mux, ax_mutex *lock, struct timeval * timeout, mux_pending_cb *pending_cb, void *arg)
{
	assert(mux);

	fd_set read_fdset, write_fdset;
	FD_ZERO(&read_fdset);
	FD_ZERO(&write_fdset);

	ax_socket fd_max = 0;
	ax_map_foreach(mux->fds.ax_map, ax_socket *, fd, int *, flags) {
#ifndef AX_OS_WIN32
		if (fd_max < *fd)
			fd_max = *fd;
#endif
		if (*flags & AX_EV_READ)
			FD_SET(*fd, &read_fdset);
		if (*flags & AX_EV_WRITE)
			FD_SET(*fd, &write_fdset);
	}

	ax_mutex_unlock(lock);
	int nreadys = select(fd_max + 1, &read_fdset, &write_fdset, NULL, timeout);
	ax_mutex_lock(lock);

	if (nreadys < 0)
		return -1;

	ax_map_foreach(mux->fds.ax_map, ax_socket *, fd, int *, flags) {
		ax_unused(flags);
		int res_flags = 0;
		if (FD_ISSET(*fd, &read_fdset))
			res_flags |= AX_EV_READ;
		if (FD_ISSET(*fd, &write_fdset))
			res_flags |= AX_EV_WRITE;

		if (res_flags) {
			pending_cb(*fd, res_flags, arg);
		}
	}

	return nreadys;
}

void mux_free(mux *mux)
{
	if (!mux)
		return;

	ax_one_free(mux->fds.ax_one);
	free(mux);
}

