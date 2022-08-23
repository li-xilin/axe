#include "polling.h"
#include "event_ht.h"
#include "ax/event/reactor.h"
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

    if((pke = realloc(pki->events, size *sizeof(struct kevent))) == NULL) {
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
    if(r == NULL) {
        ax_perror("r is null!!");
        return NULL;
    }
    
    if((ret = malloc(sizeof(struct kqueue_internal))) == NULL) {
        ax_perror("failed to malloc for kqueue_internal");
        return NULL;
    }

    memset(ret, 0, sizeof(struct kqueue_internal));
    
    if((ret->kqueue_fd = kqueue()) == -1) {
        ax_perror("failed on kqueue(): %s", strerror(errno));
        free(ret);
        return NULL;
    }

    if(kqueue_resize(ret, KQUEUE_INIT_EVENT_SIZE) == -1) {
        ax_perror("failed on kqueue_resize()");
        close(ret->kqueue_fd);
        free(ret);
        return NULL;
    }
    //ax_perror("allocated %d free events for kqeueu", KQUEUE_INIT_EVENT_SIZE);
    return ret;
}

static void kqueue_free(struct kqueue_internal *pki)
{
    assert(pki != NULL);
    if(pki == NULL) {
        ax_perror("pki is null!!");
        return;
    }

    if(pki->events) {
        free(pki->events);
        pki->events = NULL;
    }
    if(pki->kqueue_fd >= 0) {
        close(pki->kqueue_fd);
    }

    free(pki);
}

void polling_destroy(ax_reactor *r)
{
    assert(r != NULL);
    if(r == NULL) {
        ax_perror("r is null!!");
        return;
    }
    kqueue_free(r->polling_data);
}

static inline short kqueue_setup_filter(short flags)
{
    short ret = 0;
    if(flags & E_READ) {
        ret = EVFILT_READ;
    }
    if(flags & E_WRITE) {
        ret = EVFILT_WRITE;
    }
    return ret;
}

int polling_add(ax_reactor *r, ax_socket fd, short flags)
{
    struct kqueue_internal *pki;
    struct kevent e;
    int ret;
    uintptr_t ident;
    short filter;
    ushort action;

    assert(r != NULL);
    if(r == NULL) {
        ax_perror("r is null!!");
        return -1;
    } else if(flags & E_EDGE) {
        ax_perror("kqueue does not support edge-triggered mode.");
        return -1;
    }

    pki = r->polling_data;
    if(pki == NULL) {
        ax_perror("pki is null!!");
        return -1;
    }

    if(pki->nevents >= pki->max_events) {
        ax_perror("resize to %d", pki->max_events << 1);
        if(kqueue_resize(pki, pki->max_events << 1) == -1) {
            ax_perror("failed on kqueue_resize");
            return -1;
        }
    }
    ident = fd;
    filter = kqueue_setup_filter(flags);
    action = EV_ADD;
    EV_SET(&e, ident, filter, action, ((flags & E_ONCE) ? EV_ONESHOT : 0), 0, NULL);

    //ax_perror("Registering kqueue event: fd %d filter %d action %d flags %d", ident, filter, action, ((flags & E_ONCE) ? EV_ONESHOT : 0));
    ret = kevent(pki->kqueue_fd, &e, 1, NULL, 0, NULL);

    /* Error handling*/
    if(ret) {
        ax_perror("failed to add event to kqueue: %s", strerror(errno));
        return -1;
    }
    //ax_perror("Registered fd %d for envets %d", fd, flags);
    ++pki->nevents;
    return 0;
}

int polling_mod(ax_reactor *r, ax_socket fd, short flags)
{
    struct kevent e;
    int ret;
    uintptr_t ident;
    short filter;
    ushort action;

    assert(r != NULL);
    if(flags & E_EDGE) {
        ax_perror("kqueue does not support edge-triggered mode.");
        return -1;
    }

    struct kqueue_internal *pki = r->polling_data;

    ident = fd;
    filter = kqueue_setup_filter(flags);
    action = EV_ADD;
    /* EV_ADD will override the events if the fd is already registered */
    EV_SET(&e, ident, filter, action, ((flags & E_ONCE) ? EV_ONESHOT : 0), 0, NULL);

    ret = kevent(pki->kqueue_fd, &e, 1, NULL, 0, NULL);

    if(ret) {
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
    ushort action = EV_DELETE;

    struct kevent kev;
    EV_SET(&kev, ident, filter, action, 0, 0, NULL);
    
    (void)kevent(pki->kqueue_fd, &kev, 1, NULL, 0, NULL);

    pki->nevents --;
}

int polling_poll(ax_reactor *r, struct timeval *timeout)
{
    int res_flags , nreadys, i;
    ax_event *e;
    struct timespec t;

    assert(r != NULL);

    struct kqueue_internal *pki = r->polling_data;
    
    assert(pki != NULL);

    if (timeout) {
        t.tv_sec = timeout->tv_sec;
        t.tv_nsec = timeout->tv_usec * 1000;
    }
    ax_mutex_unlock(&r->lock);
    nreadys = kevent(pki->kqueue_fd, NULL, 0,
                     pki->events, pki->nevents,
                     timeout ? &t : NULL);
    ax_mutex_lock(&r->lock);
    for(i = 0; i < nreadys; ++i) {
        res_flags = 0;
        if(pki->events[i].filter == EVFILT_READ) {
            res_flags = E_READ;
        }
        if(pki->events[i].filter == EVFILT_WRITE) {
            res_flags = E_WRITE;
        }
        if(pki->events[i].flags == EV_ERROR) {
            ax_perror("kevent's EV_ERROR flag is set: %s", strerror(errno));
        }
        if(res_flags) {
            e = event_ht_retrieve(r->eht, pki->events[i].ident);
                
            assert(e != NULL);
            if(e == NULL) {
                ax_perror("the event with [fd %d] is not in the hashtable", pki->events[i].ident);
            }else{
                ax_reactor_add_to_pending(r, e, res_flags);
            }
        }
    }

    return nreadys;
}

void polling_print(ax_reactor *r)
{
}
