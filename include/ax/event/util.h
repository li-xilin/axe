#ifndef AX_EVENT_UTIL_H
#define	AX_EVENT_UTIL_H
#include "../log.h"
#include "../detect.h"
#include "../trait.h"

#ifdef AX_OS_WIN32
#include <ws2tcpip.h>
#include <winsock.h>
typedef uintptr_t ax_socket;
#else
#include <sys/time.h>
typedef int ax_socket;
#endif

extern const ax_trait ax_trait_ax_socket;

#include <stddef.h>

struct timeval;

#define ax_util_timesub(lv, rv, ov) do{\
		(ov)->tv_sec = (lv)->tv_sec - (rv)->tv_sec;\
		(ov)->tv_usec = (lv)->tv_usec - (rv)->tv_usec;\
		if((ov)->tv_usec < 0) {\
			(ov)->tv_usec += 1000000;\
			(ov)->tv_sec--;\
		}\
	} while(0);

int ax_util_socketpair(int family, int type, int protocol, ax_socket fd[2]);

int ax_util_close_fd(ax_socket fd);

int ax_util_timeofday(struct timeval * tv);

int ax_util_set_nonblocking(ax_socket fd);

#endif
