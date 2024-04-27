#define __BSD_VISIBLE 1

#include "../mux.h"
#include "ax/log.h"

#include <sys/types.h>
#include <sys/event.h>
#include <unistd.h>
#include <assert.h>
#include <memory.h>
#include <stdlib.h>
#include <errno.h>

#define KQUEUE_INIT_EVENT_SIZE 32

struct mux_st
{
	int kqueue_fd;
	int nevents;
	int max_events;
	struct kevent *events;
};

static int kqueue_resize(struct mux_st *mux, int size)
{
	struct kevent *pke;
	assert(mux != NULL);

	if ((pke = realloc(mux->events, size * sizeof(struct kevent))) == NULL) {
		ax_perror("failed to realloc for events, maybe run out of memory.");
		return -1;
	}

	mux->events = pke;
	mux->max_events = size;
	return 0;
}

mux *mux_init(void)
{
	struct mux_st *ret;

	if ((ret = malloc(sizeof(struct mux_st))) == NULL) {
		ax_perror("failed to malloc for mux_st");
		return NULL;
	}

	memset(ret, 0, sizeof(struct mux_st));

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

void mux_free(mux *mux)
{
	if (!mux)
		return;

	if (mux->events) {
		free(mux->events);
		mux->events = NULL;
	}

	if (mux->kqueue_fd >= 0) {
		close(mux->kqueue_fd);
	}

	free(mux);
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

int mux_add(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	if (mux->nevents >= mux->max_events) {
		ax_perror("resize to %d", mux->max_events << 1);
		if (kqueue_resize(mux, mux->max_events << 1) == -1) {
			ax_perror("failed on kqueue_resize");
			return -1;
		}
	}

	uintptr_t ident = fd;
	short filter = kqueue_setup_filter(flags);
	unsigned short action = EV_ADD;

	struct kevent e;
	EV_SET(&e, ident, filter, action, ((flags & AX_EV_ONCE) ? EV_ONESHOT : 0), 0, NULL);

	int ret = kevent(mux->kqueue_fd, &e, 1, NULL, 0, NULL);

	if (ret) {
		ax_perror("failed to add event to kqueue: %s", strerror(errno));
		return -1;
	}
	++mux->nevents;
	return 0;
}

int mux_mod(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	uintptr_t ident = fd;
	short filter = kqueue_setup_filter(flags);
	unsigned short action = EV_ADD;
	/* EV_ADD will override the events if the fd is already registered */

	struct kevent e;
	EV_SET(&e, ident, filter, action, ((flags & AX_EV_ONCE) ? EV_ONESHOT : 0), 0, NULL);

	int ret = kevent(mux->kqueue_fd, &e, 1, NULL, 0, NULL);

	if (ret) {
		ax_perror("failed to modify the event: %s", strerror(errno));
		return -1;
	}
	return 0;
}

void mux_del(mux *mux, ax_socket fd, short flags)
{
	assert(mux != NULL);

	uintptr_t ident = fd;
	short filter = kqueue_setup_filter(flags);
	unsigned short action = EV_DELETE;

	struct kevent kev;
	EV_SET(&kev, ident, filter, action, 0, 0, NULL);

	(void)kevent(mux->kqueue_fd, &kev, 1, NULL, 0, NULL);

	mux->nevents --;
}

int mux_poll(mux *mux, ax_lock *lock, struct timeval * timeout, mux_pending_cb *pending_cb, void *arg)
{
	assert(mux != NULL);
	int res_flags , nreadys, i;

	struct timespec *spec = timeout
		? (struct timespec[]){{ .tv_sec = timeout->tv_sec,
				.tv_nsec = timeout->tv_usec * 1000 }}
		: NULL;
	ax_lock_put(lock);
	nreadys = kevent(mux->kqueue_fd, NULL, 0, mux->events, mux->nevents, spec);
	ax_lock_get(lock);
	for (i = 0; i < nreadys; ++i) {
		res_flags = 0;
		if (mux->events[i].filter == EVFILT_READ)
			res_flags = AX_EV_READ;
		if (mux->events[i].filter == EVFILT_WRITE)
			res_flags = AX_EV_WRITE;
		if (mux->events[i].flags == EV_ERROR)
			res_flags = AX_EV_ERROR;

		if (res_flags)
			pending_cb(mux->events[i].ident, res_flags, arg);
	}

	return nreadys;
}

