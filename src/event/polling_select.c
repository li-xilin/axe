#include "event_ht.h"
#include "polling.h"
#include "ax/event/util.h"
#include "ax/event/event.h"
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


struct select_internal
{
	int n_readfd, n_writefd;
	ax_hmap_r fds;
};

void *polling_init(ax_reactor *r)
{
	struct select_internal *si = NULL;
	si = malloc(sizeof *si);
	if (!si)
		return NULL;
	
	si->fds.ax_one = NULL;
	si->n_readfd = si->n_writefd = 0;

	si->fds = ax_new(ax_hmap, ax_t(ax_socket), ax_t(short));
	if (ax_r_isnull(si->fds))
		goto fail;
	return si;
fail:
	if (si) {
		ax_one_free(si->fds.ax_one);
		free(si);
	}
	return NULL;
}

static void select_internal_free(struct select_internal *si)
{
	if (si) {
		ax_one_free(si->fds.ax_one);
		free(si);
	}
}

int polling_add(ax_reactor *r, ax_socket fd, short flags)
{
	assert(r != NULL);

	struct select_internal *psi = r->polling_data;

	if (flags & AX_EV_EDGE) {
		ax_perror("Edge-trigger flag is not supported");
		return -1;
	}

	if (psi->n_readfd + (flags & AX_EV_READ) == FD_SETSIZE
			|| psi->n_writefd + (flags & AX_EV_WRITE) == FD_SETSIZE) {
		ax_perror("Number of fd exceeds the limit");
		return -1;
	}

	if (flags & AX_EV_READ)
		psi->n_readfd ++;
	if (flags & AX_EV_WRITE)
		psi->n_writefd ++;

	short flags_insert = flags & (AX_EV_READ|AX_EV_WRITE);
	if (ax_map_exist(psi->fds.ax_map, &fd))
		return -1;
	if (!ax_map_put(psi->fds.ax_map, &fd, &flags_insert))
		return -1;

	return 0;
}

int polling_mod(ax_reactor *r, ax_socket fd, short flags)
{
	assert(r != NULL);

	struct select_internal *psi = r->polling_data;

	if (flags & AX_EV_EDGE) {
		ax_pinfo("Edge-trigger flag is not supported");
		return -1;
	}

	ax_iter it = ax_map_at(psi->fds.ax_map, &fd);
	if (ax_box_iter_ended(psi->fds.ax_box, &it))
		return -1;

	short *pflags = ax_iter_get(&it);

	int read_changed = (*pflags ^ flags) & AX_EV_READ ? ((flags & AX_EV_READ) << 1) - 1 : 0;
	int write_changed = (*pflags ^ flags) & AX_EV_WRITE ? ((flags & AX_EV_WRITE) << 1) - 1 : 0;

	if (psi->n_readfd + read_changed == FD_SETSIZE
			|| psi->n_writefd + write_changed == FD_SETSIZE) {
		ax_perror("Number of fd exceeds the limit");
		return -1;
	}
	psi->n_readfd += read_changed;
	psi->n_writefd += write_changed;

	*pflags = flags;
	return 0;
}

void polling_del(ax_reactor *r, ax_socket fd, short flags)
{
	assert(r != NULL);
	
	struct select_internal *psi = r->polling_data;

	short old_flags= *(short *)ax_map_get(psi->fds.ax_map, &fd);

	if ((old_flags & AX_EV_READ) && (flags & AX_EV_READ))
		psi->n_readfd--;
	if ((old_flags & AX_EV_WRITE) && (flags & AX_EV_WRITE))
		psi->n_readfd--;

	short new_flags = old_flags = old_flags & (~flags);
	if (!(new_flags & (AX_EV_READ|AX_EV_WRITE)))
		ax_map_erase(psi->fds.ax_map, &fd);
}

int polling_poll(ax_reactor *r, struct timeval *timeout)
{

	assert(r != NULL);

	struct select_internal *psi = r->polling_data;

	fd_set read_fdset, write_fdset;
	FD_ZERO(&read_fdset);
	FD_ZERO(&write_fdset);

	ax_socket fd_max = 0;
	ax_map_foreach(psi->fds.ax_map, ax_socket *, fd, int *, flags) {
#ifndef AX_OS_WIN32
		if (fd_max < *fd)
			fd_max = *fd;
#endif
		if (*flags & AX_EV_READ)
			FD_SET(*fd, &read_fdset);
		if (*flags & AX_EV_WRITE)
			FD_SET(*fd, &write_fdset);
	}

	ax_mutex_unlock(&r->lock);
	int nreadys = select(fd_max + 1, &read_fdset, &write_fdset, NULL, timeout);
	ax_mutex_lock(&r->lock);

	if (nreadys < 0)
		return -1;
	
	ax_map_foreach(psi->fds.ax_map, ax_socket *, fd, int *, flags) {
		ax_unused(flags);
		int res_flags = 0;
		if (FD_ISSET(*fd, &read_fdset))
			res_flags |= AX_EV_READ;
		if (FD_ISSET(*fd, &write_fdset))
			res_flags |= AX_EV_WRITE;

		if (res_flags) {
			ax_event *e = event_ht_retrieve(r->eht, *fd);
			ax_reactor_add_to_pending(r, e, res_flags);
		}
	}

	return nreadys;
}

void polling_destroy(ax_reactor *r)
{
	assert(r != NULL);

	select_internal_free(r->polling_data);
}

void polling_print(ax_reactor *r)
{
	
}
