#define __BSD_VISIBLE 1

#include "polling.h"
#include "event_ht.h"
#include "reactor_type.h"
#include "ax/reactor.h"
#include "ax/timeval.h"
#include "ax/log.h"

#include <sys/types.h>
#include <sys/event.h>
#include <unistd.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>

#define KQUEUE_INIT_EVENT_SIZE 32

struct kqueue_internal
{
	int kqueue_fd;
	int nevents;
	int max_events;
	struct kevent *events;
};

static int kqueue_resize(struct kqueue_internal *pki, int size)
{
	struct kevent *pke;
	assert(pki != NULL);

	if ((pke = realloc(pki->events, size * sizeof(struct kevent))) == NULL) {
		ax_perror("failed to realloc for events, maybe run out of memory.");
		return -1;
	}

	pki->events = pke;
	pki->max_events = size;
	return 0;
}

void *polling_init(ax_reactor *r)
{
	struct kqueue_internal *ret;

	assert(r);

	if ((ret = malloc(sizeof(struct kqueue_internal))) == NULL) {
		ax_perror("failed to malloc for kqueue_internal");
		return NULL;
	}

	memset(ret, 0, sizeof(struct kqueue_internal));

	if ((ret->kqueue_fd = kqueue()) == -1) {
		ax_perror("failed on kqueue(): %s", strerror(errno));
		free(ret);
		return NULL;
	}

	if (kqueue_resize(ret, KQUEUE_INIT_EVENT_SIZE) == -1) {
		ax_perror("failed on kqueue_resize()");
		close(ret->kqueue_fd);
		free(ret);
		return NULL;
	}
	return ret;
}

static void kqueue_free(struct kqueue_internal *pki)
{
	assert(pki != NULL);

	if (pki->events) {
		free(pki->events);
		pki->events = NULL;
	}

	if (pki->kqueue_fd >= 0) {
		close(pki->kqueue_fd);
	}

	free(pki);
}

void polling_destroy(ax_reactor *r)
{
	assert(r != NULL);
	if (r == NULL) {
		ax_perror("r is null!!");
		return;
	}
	kqueue_free(r->polling_data);
}

static inline short kqueue_setup_filter(short flags)
{
	short ret = 0;
	if (flags & AX_EV_READ)
		ret = EVFILT_READ;
	if (flags & AX_EV_WRITE)
		ret = EVFILT_WRITE;
	return ret;
}

int polling_add(ax_reactor *r, ax_socket fd, short flags)
{

	assert(r != NULL);
	if (flags & AX_EV_EDGE) {
		ax_perror("kqueue does not support edge-triggered mode.");
		return -1;
	}

	struct kqueue_internal *pki = r->polling_data;
	if (!pki) {
		ax_perror("pki is null!!");
		return -1;
	}

	if (pki->nevents >= pki->max_events) {
		ax_perror("resize to %d", pki->max_events << 1);
		if (kqueue_resize(pki, pki->max_events << 1) == -1) {
			ax_perror("failed on kqueue_resize");
			return -1;
		}
	}
	uintptr_t ident = fd;
	short filter = kqueue_setup_filter(flags);
	unsigned short action = EV_ADD;

	struct kevent e;
	EV_SET(&e, ident, filter, action, ((flags & AX_EV_ONCE) ? EV_ONESHOT : 0), 0, NULL);

	int ret = kevent(pki->kqueue_fd, &e, 1, NULL, 0, NULL);

	if (ret) {
		ax_perror("failed to add event to kqueue: %s", strerror(errno));
		return -1;
	}
	++pki->nevents;
	return 0;
}

int polling_mod(ax_reactor *r, ax_socket fd, short flags)
{
	assert(r != NULL);
	if (flags & AX_EV_EDGE) {
		ax_perror("kqueue does not support edge-triggered mode.");
		return -1;
	}

	struct kqueue_internal *pki = r->polling_data;
	uintptr_t ident = fd;
	short filter = kqueue_setup_filter(flags);
	unsigned short action = EV_ADD;
	/* EV_ADD will override the events if the fd is already registered */

	struct kevent e;
	EV_SET(&e, ident, filter, action, ((flags & AX_EV_ONCE) ? EV_ONESHOT : 0), 0, NULL);

	int ret = kevent(pki->kqueue_fd, &e, 1, NULL, 0, NULL);

	if (ret) {
		ax_perror("failed to modify the event: %s", strerror(errno));
		return -1;
	}
	return 0;
}

void polling_del(ax_reactor *r, ax_socket fd, short flags)
{
	assert(r != NULL);

	struct kqueue_internal *pki = r->polling_data;
	uintptr_t ident = fd;
	short filter = kqueue_setup_filter(flags);
	unsigned short action = EV_DELETE;

	struct kevent kev;
	EV_SET(&kev, ident, filter, action, 0, 0, NULL);

	(void)kevent(pki->kqueue_fd, &kev, 1, NULL, 0, NULL);

	pki->nevents --;
}

int polling_poll(ax_reactor *r, struct timeval *timeout)
{
	int res_flags , nreadys, i;

	assert(r != NULL);

	struct kqueue_internal *pki = r->polling_data;

	assert(pki != NULL);

	struct timespec *spec = timeout
		? (struct timespec[]){{ .tv_sec = timeout->tv_sec,
				.tv_nsec = timeout->tv_usec * 1000 }}
		: NULL;
	ax_mutex_unlock(&r->lock);
	nreadys = kevent(pki->kqueue_fd, NULL, 0, pki->events, pki->nevents, spec);
	ax_mutex_lock(&r->lock);
	for (i = 0; i < nreadys; ++i) {
		res_flags = 0;
		if (pki->events[i].filter == EVFILT_READ)
			res_flags = AX_EV_READ;
		if (pki->events[i].filter == EVFILT_WRITE)
			res_flags = AX_EV_WRITE;
		if (pki->events[i].flags == EV_ERROR)
			ax_perror("kevent's EV_ERROR flag is set: %s", strerror(errno));

		if (res_flags) {
			ax_event *e = event_ht_retrieve(r->eht, pki->events[i].ident);
			ax_reactor_add_to_pending(r, e, res_flags);
		}
	}

	return nreadys;
}

void polling_print(ax_reactor *r)
{
}
