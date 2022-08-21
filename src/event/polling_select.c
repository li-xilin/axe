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
	ax_hmap_r fds;
};

void *polling_init(ax_reactor * r)
{
	struct select_internal *si = NULL;
	si = malloc(sizeof *si);
	if (!si)
		return NULL;
	
	si->fds.ax_one = NULL;

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

int polling_add(ax_reactor * r, ax_socket fd, short flags)
{
	struct select_internal * psi;
	assert(r != NULL);

	psi = r->polling_data;

	short flags_insert = flags & (E_READ|E_WRITE);
	ax_map_put(psi->fds.ax_map, &fd, &flags_insert);

	if (flags & E_EDGE) {
		ax_perror("ET is not supported by select polling policy.");
	}
	return (0);
}

int polling_mod(ax_reactor * r, ax_socket fd, short flags)
{
	/*
	struct select_internal * psi;
	assert(r != NULL);
	if (r == NULL) {
		ax_perror("r is null!!");
		return (-1);
	}
	ax_perror("Adding a event with fd %d and flags %d", fd, flags);

	psi = r->polling_data;
	if (psi == NULL) {
		ax_perror("pei is null!!");
		return (-1);
	}

	if (fd > psi->maxfd) {
		ax_perror("fd[%d] is not in the select set", fd);
		return (-1);
	}

	if (flags & E_READ) {
		FD_SET(fd, psi->readset_in);
	} else {
		FD_CLR(fd, psi->readset_in);
	}

	if (flags & E_WRITE) {
		FD_SET(fd, psi->writeset_in);
	} else {
		FD_CLR(fd, psi->writeset_in);
	}
	
	if (flags & E_EDGE) {
		ax_perror("ET is not supported by select polling policy.");
	}
	ax_perror("Modified the event with fd %d and flags %d", fd, flags);
	*/
	return 0;
}

int polling_del(ax_reactor * r, ax_socket fd, short flags)
{
	struct select_internal * psi;

	assert(r != NULL);
	
	psi = r->polling_data;

	 short *pflags= ax_map_get(psi->fds.ax_map, &fd);

	*pflags = *pflags & (~flags);
	if (*pflags == 0)
		ax_map_erase(psi->fds.ax_map, &fd);
	
	return 0;
}

int polling_poll(ax_reactor * r, struct timeval * timeout)
{
	ax_event * e;

	assert(r != NULL);

	struct select_internal * psi = r->polling_data;

	fd_set read_fdset, write_fdset;
	FD_ZERO(&read_fdset);
	FD_ZERO(&write_fdset);

	ax_socket fd_max = 0;
	ax_map_foreach(psi->fds.ax_map, ax_socket *, fd, int *, flags) {
#ifndef AX_OS_WIN32
		if (fd_max < *fd)
			fd_max = *fd;
#endif
		if (*flags & E_READ)
			FD_SET(*fd, &read_fdset);
		if (*flags & E_WRITE)
			FD_SET(*fd, &write_fdset);
	}

	ax_mutex_unlock(&r->lock);
	int nreadys = select(fd_max + 1, &read_fdset, &write_fdset, NULL, timeout);
	ax_mutex_lock(&r->lock);
	
	if (nreadys) {
		ax_map_foreach(psi->fds.ax_map, ax_socket *, fd, int *, flags) {
			ax_unused(flags);
			int res_flags = 0;
			if (FD_ISSET(*fd, &read_fdset))
				res_flags |= E_READ;
			if (FD_ISSET(*fd, &write_fdset))
				res_flags |= E_WRITE;

			if (res_flags) {
				e = event_ht_retrieve(r->eht, *fd);
				
				assert(e != NULL);
				if (e == NULL) {
					ax_perror("the event with [fd %d] is not in the hashtable", *fd);
				}else{
					ax_reactor_add_to_pending(r, e, res_flags);
				}
			}
		}
	}
	

	return nreadys;
}

void polling_destroy(ax_reactor * r)
{
	assert(r != NULL);

	select_internal_free(r->polling_data);
}

void polling_print(ax_reactor * r)
{
	
}
