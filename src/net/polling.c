#include "polling.h"

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

#if defined(HAVE_EPOLL)
#include "polling_epoll.c"
#elif defined(HAVE_KQUEUE)
#include "polling_kqueue.c"
#elif defined(HAVE_POLL)
#include "polling_poll.c"
#elif defined(HAVE_SELECT)
#include "polling_select.c"
#endif
