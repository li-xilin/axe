#ifndef AX_EVENT_UTIL_H
#define	AX_EVENT_UTIL_H
#include "../log.h"
#include "../detect.h"
#include "../trait.h"

#ifdef AX_OS_WIN32
typedef uintptr_t ax_socket;
#else
typedef int ax_socket;
#endif

struct timeval;

extern const ax_trait ax_trait_ax_socket;

#include <stddef.h>


int ax_util_socketpair(int family, int type, int protocol, ax_socket fd[2]);

int ax_util_close_fd(ax_socket fd);

int ax_util_set_nonblocking(ax_socket fd);

#endif
