#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <assert.h>

#include "ax/event/util.h"
#include "ax/event/event.h"
#include "event_ht.h"
#include "polling.h"

#ifdef AX_OS_WIN32
	#include <windows.h>
	#include <Winsock2.h>
#else
	#include <sys/select.h>
#endif

typedef unsigned long FD_MASK;

#define NBPB 8  /* Number of bits per byte */
#define NBPM (sizeof(FD_MASK) * NBPB) /* Number of bits per mask */
#define DIV_ROUND_UP(x, y) (((x) + (y - 1)) / (y))/* Rounding up division */
#define FD_TO_NUM_OF_INT(fd) DIV_ROUND_UP(fd, NBPM)/* Number of int needed to hold the given fd */
#define FD_TO_BYTES(fd) (FD_TO_NUM_OF_INT(fd) * sizeof(unsigned long))/* Number of bytes needed to hold the given fd */
#define FD_INIT_BYTES 32	/* Initial fd_set size */
struct select_internal
{
	ax_socket maxfd;
	int bytes_avail;
	fd_set *readset_in;
	fd_set *writeset_in;
	fd_set *readset_out;
	fd_set *writeset_out;
};

static int select_resize(struct select_internal * psi, int bytes)
{
	fd_set * preadset_in = NULL;
	fd_set * pwriteset_in = NULL;
	fd_set * preadset_out = NULL;
	fd_set * pwriteset_out = NULL;

	assert(psi != NULL);
	if (psi == NULL) {
		ax_perror("psi is null!!");
		return (-1);
	}
	if ((preadset_in = realloc(psi->readset_in, bytes)) == NULL) {
		return (-1);
	}
	if ((pwriteset_in = realloc(psi->writeset_in, bytes)) == NULL) {
		free(preadset_in);
		return (-1);
	}
	if ((preadset_out= realloc(psi->readset_out, bytes)) == NULL) {
		free(preadset_in);
		free(pwriteset_in);
		return (-1);
	}
	if ((pwriteset_out = realloc(psi->writeset_out, bytes)) == NULL) {
		free(preadset_in);
		free(pwriteset_in);
		free(preadset_out);
		return (-1);
	}
	psi->readset_in		= preadset_in;
	psi->writeset_in 	= pwriteset_in;
	psi->readset_out 	= preadset_out;
	psi->writeset_out 	= pwriteset_out;
	/* 
	* Zeros the newly appended memory.
	* We don't have to zeros the outs, because they will copy from the ins.
	*/
	memset(psi->readset_in + psi->bytes_avail, 0, bytes - psi->bytes_avail);
	memset(psi->writeset_in + psi->bytes_avail, 0, bytes - psi->bytes_avail);
	/*  */

	psi->bytes_avail = bytes;

	return (0);
}

static int select_free(struct select_internal * psi)
{
	assert(psi != NULL);

	if (psi->readset_in)
		free(psi->readset_in);
	if (psi->writeset_in)
		free(psi->writeset_in);
	if (psi->readset_out)
		free(psi->readset_out);
	if (psi->writeset_out)
		free(psi->writeset_out);
	psi->readset_in = psi->writeset_in = NULL;
	psi->readset_out = psi->writeset_out = NULL;
	free(psi);
	ax_perror("done freeing policy_select internal data.");
	return (0);
}

/*
* Create and initialize the internal data used by select polling policy.
* Return value: newly created internal data on success, NULL on failure.
* @r: the reactor which uses this policy.
*/
void *polling_init(ax_reactor * r)
{
	struct select_internal * ret;

	assert(r != NULL);
	if (r == NULL) {
		ax_perror("r is null!!");
		return NULL;
	}

	if ((ret = malloc(sizeof(struct select_internal))) == NULL) {
		return NULL;
	}

	memset(ret, 0, sizeof(struct select_internal));

	if (select_resize(ret, FD_TO_BYTES(FD_INIT_BYTES + 1)) == -1) {
		select_free(ret);
		return NULL;
	}

	return ret;
}

int polling_add(ax_reactor * r, ax_socket fd, short flags)
{
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
		int new_avail = psi->bytes_avail;

		//Doubles up to accommodate the fd
		while (new_avail < FD_TO_BYTES(fd + 1))
			new_avail <<= 1;

		if (new_avail != psi->bytes_avail) {
			if (select_resize(psi, new_avail) == -1) {
				ax_perror("memory shortage.");
				return (-1);
			}
		}
		//maintain the maximum fd for the select syscall
		psi->maxfd = fd;
	}

	if (flags & E_READ) {
		FD_SET(fd, psi->readset_in);
	}
	if (flags & E_WRITE) {
		FD_SET(fd, psi->writeset_in);
	}
	if (flags & E_EDGE) {
		ax_perror("ET is not supported by select polling policy.");
	}
	ax_perror("Added a event with fd %d and flags %d", fd, flags);
	return (0);
}

int polling_mod(ax_reactor * r, ax_socket fd, short flags)
{
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
	return (0);
}

int polling_del(ax_reactor * r, ax_socket fd, short flags)
{
	struct select_internal * psi;

	assert(r != NULL);
	ax_perror("Removing a event with fd %d and flags %d in the fd_set", fd, flags);
	
	psi = r->polling_data;

	assert(psi != NULL);
	if (psi == NULL) {
		ax_perror("policy internal data is needed but null provided.");
		return (-1);
	}
	if (fd > psi->maxfd) return(0);

	if (flags & E_READ) {
		FD_CLR(fd, psi->readset_in);
	}
	if (flags & E_WRITE) {
		FD_CLR(fd, psi->writeset_in);
	}
	ax_perror("Removed a event with fd %d and flags %d in the fd_set", fd, flags);
	return (0);
}

int polling_poll(ax_reactor * r, struct timeval * timeout)
{
	int res_flags , nreadys, fd;
	struct select_internal * psi;
	ax_event * e;

	assert(r != NULL);

	psi = r->polling_data;
	
	assert(psi != NULL);

	memcpy(psi->readset_out, psi->readset_in, psi->bytes_avail);
	memcpy(psi->writeset_out, psi->writeset_in, psi->bytes_avail);

	ax_mutex_unlock(&r->lock);
	nreadys = select(psi->maxfd + 1, psi->readset_out, psi->writeset_out, NULL, timeout);
	ax_mutex_lock(&r->lock);
	
	if (nreadys) {
		for (fd = 0; fd <= psi->maxfd; ++fd) {
			res_flags = 0;
			if (FD_ISSET(fd, psi->readset_out) && FD_ISSET(fd, psi->readset_in))res_flags |= E_READ;;
			if (FD_ISSET(fd, psi->writeset_out) && FD_ISSET(fd, psi->readset_in))res_flags |= E_WRITE;

			if (res_flags) {
				e = event_ht_retrieve(r->eht, fd);
				
				assert(e != NULL);
				if (e == NULL) {
					ax_perror("the event with [fd %d] is not in the hashtable", fd);
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

	select_free(r->polling_data);
}

void polling_print(ax_reactor * r)
{
	int i;
	struct select_internal * psi = r->polling_data;

	assert(r != NULL);

	psi = r->polling_data;
	
	printf("maxfd:%d\n", psi->maxfd);
	printf("bytes_avail:%d\n", psi->bytes_avail);
	for (i = 0; i <= psi->maxfd; ++i) {
		printf("%d", i);
		if (FD_ISSET(i, psi->readset_in)) {
			printf(" set for reading");
		}
		if (FD_ISSET(i, psi->writeset_in)) {
			printf(" set for writing");
		}
		printf("\n");
	}
}
