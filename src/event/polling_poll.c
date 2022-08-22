#include <sys/errno.h>
#include <poll.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>

#include "polling.h"
#include "event_ht.h"
#include "ax/log.h"

#include <stdio.h>

#define POLL_INIT_EVENT_SIZE 32

struct poll_internal
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

static int poll_resize(struct poll_internal * ppi, int size)
{
	struct pollfd * pfds;
	assert(ppi != NULL);
	if(ppi == NULL) {
		ax_perror("ppi is null!!");
		return (-1);
	}

	if((pfds = realloc(ppi->fds_in, size * sizeof(struct pollfd))) == NULL) {
		ax_perror("failed to realloc for fds, maybe run out of memory.");
		return (-1);
	}

	ppi->fds_in = pfds;
	ppi->max_events = size;
	ppi->need_realoc = 1;
	ppi->need_rememcp = 1;
	return (0);
}

void * polling_init(ax_reactor * r)
{
	struct poll_internal * ret;
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return NULL;
	}

	if((ret = malloc(sizeof(struct poll_internal))) == NULL) {
		ax_perror("failed to malloc for poll_internal");
		return NULL;
	}

	memset(ret, 0, sizeof(struct poll_internal));

	if(poll_resize(ret, POLL_INIT_EVENT_SIZE) == -1) {
		ax_perror("failed on poll_resize");
		free(ret);
		return NULL;
	}

	return ret;
}

static void poll_free(struct poll_internal * ppi)
{
	assert(ppi != NULL);
	if(ppi == NULL) {
		ax_perror("ppi is null!!");
		return;
	}

	if(ppi->fds) {
		free(ppi->fds);
		ppi->fds = NULL;
	}
	if(ppi->fds_in) {
		free(ppi->fds_in);
		ppi->fds_in = NULL;
	}

	free(ppi);
}

void polling_destroy(ax_reactor * r)
{
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return;
	}
	poll_free(r->polling_data);
}

static inline int poll_setup_mask(short flags)
{
	int ret = 0;
	if(flags & E_READ) {
		ret |= POLLIN | POLLPRI;
	}
	if(flags & E_WRITE) {
		ret |= POLLOUT;
	}
	return ret;
}

int polling_add(ax_reactor * r, ax_socket fd, short flags)
{
	struct poll_internal * ppi;
	int i;
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return (-1);
	}

	ppi = r->polling_data;
	if(ppi == NULL) {
		ax_perror("ppi is null!!");
		return (-1);
	}

	for(i = 0; i < ppi->n_events; ++i) {
		if(fd == ppi->fds_in[i].fd) {
			/* Override the existing event */
			ppi->fds_in[i].events = poll_setup_mask(flags);
			return (0);
		}
	}
	if(ppi->n_events >= ppi->max_events) {
		ax_perror("resize to %d", ppi->max_events << 1);
		if(poll_resize(ppi, ppi->max_events << 1) == -1) {
			ax_perror("failed on poll_resize");
			return (-1);
		}
	}
	ppi->fds_in[i].fd = fd;
	ppi->fds_in[i].events = poll_setup_mask(flags);
	ppi->need_rememcp = 1;;
	++ppi->n_events;

	return (0);
}

int polling_mod(ax_reactor * r, ax_socket fd, short flags)
{
	struct poll_internal * ppi;
	int i;
	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return (-1);
	}

	ppi = r->polling_data;
	if(ppi == NULL) {
		ax_perror("ppi is null!!");
		return (-1);
	}

	for(i = 0; i < ppi->n_events; ++i) {
		if(fd == ppi->fds_in[i].fd) {
			/* Override the existing event */
			ppi->fds_in[i].events = poll_setup_mask(flags);
			return (0);
		}
	}

	ax_perror("fd[%d] is not in the poll fdset.", fd);
	return (-1);
}

static inline void poll_swap_pollfd(struct pollfd * lhs, struct pollfd * rhs)
{
	struct pollfd t;
	t = *lhs;
	*lhs = *rhs;
	*rhs = t;
}

int polling_del(ax_reactor * r, ax_socket fd, short flags)
{
	struct poll_internal * ppi;
	int i;

	assert(r != NULL);
	if(r == NULL) {
		ax_perror("r is null!!");
		return (-1);
	}
	ppi = r->polling_data;

	assert(ppi != NULL);
	if(ppi == NULL) {
		ax_perror("policy internal data is needed but null provided.");
		return (-1);
	}
	for(i = 0; i < ppi->n_events; ++i) {
		if(ppi->fds_in[i].fd == fd) {
			poll_swap_pollfd(&ppi->fds_in[i], &ppi->fds_in[ppi->n_events - 1]);
			ppi->need_rememcp = 1;
			--ppi->n_events;
			return (0);
		}
	}

	ax_perror("the [fd %d] in not in the pollfds", fd);

	return (-1);
}

int polling_poll(ax_reactor * r, struct timeval * timeout)
{
	int res_flags , nreadys;
	struct poll_internal * ppi;
	ax_event * e;
	int i;

	assert(r != NULL);

	ppi = r->polling_data;

	assert(ppi != NULL);

		
	if(ppi->need_realoc) {
		if((ppi->fds = realloc(ppi->fds, ppi->max_events * sizeof(struct pollfd))) == NULL) {
			ax_perror("failed to realloc for ppi->fds");
			exit(1);
		}
		ppi->need_realoc = 0;
	}
	if(ppi->need_rememcp) {
		memcpy(ppi->fds, ppi->fds_in, ppi->max_events * sizeof(struct pollfd));
		ppi->need_rememcp = 0;
	}
		/* No need to realloc and rememcpy since we are in single-threaeded environment. */
		/* ppi->fds = ppi->fds_in; */

	ax_mutex_unlock(&r->lock);
	nreadys = poll(ppi->fds, 
			ppi->n_events, 
			timeout ? timeout->tv_sec * 1000 + timeout->tv_usec / 1000 : -1);
	ax_mutex_lock(&r->lock);

	if(nreadys) {
		for(i = 0; i < ppi->n_events; ++i) {
			res_flags = 0;
			if(ppi->fds[i].revents & (POLLIN | POLLPRI)) {
				res_flags |= E_READ;
			}
			if(ppi->fds[i].revents & POLLOUT) {
				res_flags |= E_WRITE;
			}
			if(ppi->fds[i].revents & POLLNVAL) {
				ax_perror("[fd %d] is invalid!", ppi->fds[i].fd);
			}
			if(ppi->fds[i].revents & POLLERR) {
				ax_perror("got a POLLERR event: %s", strerror(errno));
			}

			if(res_flags) {
				e = event_ht_retrieve(r->eht, ppi->fds[i].fd);

				assert(e != NULL);
				if(e == NULL) {
					ax_perror("the event with [fd %d] is not in the hashtable", ppi->fds[i].fd);
				}else{
					ax_reactor_add_to_pending(r, e, res_flags);
				}
			}
		}
	}
	return nreadys;
}

void polling_print(ax_reactor * r)
{
	int i;
	struct poll_internal * ppi = r->polling_data;

	assert(r != NULL);

	ppi = r->polling_data;

	printf("max_events:%d\n", ppi->max_events);
	printf("n_events:%d\n", ppi->n_events);
	printf("fds:%p\n", (void *)ppi->fds);
	printf("fds_in:%p\n", (void *)ppi->fds_in);
	for(i = 0; i < ppi->n_events; ++i) {
		printf("[fd %d]: ", (int)ppi->fds[i].fd);
		if(ppi->fds[i].revents & (POLLIN | POLLPRI)) {
			printf("set for reading ");
		}
		if(ppi->fds[i].revents & POLLOUT) {
			printf("set for writing ");
		}
		printf("\n");
	}
}
