#include "config.h"
#include "ax/event/skutil.h"

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

void * polling_init(ax_reactor * r);
int polling_add(ax_reactor * r, ax_socket fd, short flags);
int polling_del(ax_reactor * r, ax_socket fd, short flags);
int polling_poll(ax_reactor * r, struct timeval * timeout);
void polling_destroy(ax_reactor * r);
void polling_print(ax_reactor * r);

#if defined(HAVE_EPOLL)
#include "polling_epoll.c"
#elif defined(HAVE_KQUEUE)
#include "polling_kqueue.c"
#elif defined(HAVE_POLL)
#include "polling_poll.c"
#elif defined(HAVE_SELECT)
#include "polling_select.c"
#endif
