#ifndef AX_EVENT_SKUTIL_H
#define	AX_EVENT_SKUTIL_H
#include "../log.h"
#include "../detect.h"

#ifdef AX_OS_WIN32
#include "winsock.h"
#define ax_socket intptr_t
#else
#include "sys/time.h"
#define ax_socket int
#endif

struct timeval;

#define ax_util_timesub(lv, rv, ov) do{\
		(ov)->tv_sec = (lv)->tv_sec - (rv)->tv_sec;\
		(ov)->tv_usec = (lv)->tv_usec - (rv)->tv_usec;\
		if((ov)->tv_usec < 0) {\
			(ov)->tv_usec += 1000000;\
			(ov)->tv_sec--;\
		}\
	} while(0);

int ax_util_create_pipe(ax_socket pipe[2]);

int ax_util_close_fd(ax_socket fd);

int ax_util_timeofday(struct timeval * tv);

int ax_util_set_nonblocking(ax_socket fd);

#endif
