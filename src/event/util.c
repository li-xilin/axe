#include "ax/event/util.h"
#include "ax/log.h"

#ifdef AX_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#endif

#include <errno.h>
#include <string.h>

inline int ax_util_create_pipe(ax_socket p[2])
{
	#ifdef AX_OS_WIN32
		return - !!CreatePipe(&p[0], &p[1], NULL, 0);
	#else
		return pipe(p);
	#endif
}

inline int ax_util_close_fd(ax_socket fd) {
	#ifdef AX_OS_WIN32
		return closesocket(fd) ? -1 : 0;
	#else
		return close(fd);
	#endif
}

int ax_util_timeofday(struct timeval * tv)
{
	#ifdef AX_OS_WIN32
		union {
			long long ns100;
			FILETIME ft;
		} u_time;
		GetSystemTimeAsFileTime (&u_time.ft);
	    tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	    tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
	    return (0);
	#else
	    return gettimeofday(tv, NULL);
	#endif
}

int ax_util_set_nonblocking(ax_socket fd)
{
	#ifdef AX_OS_WIN32
		ULONG nonBlock = 1;
		if (ioctlsocket(m_Socket, FIONBIO, &nonBlock)) {
			ax_perror("failed on ioctlsocket");
			return (-1);
		}
		return (0);
	#else
		int opts;

		if ((opts = fcntl(fd, F_GETFL)) < 0) {
			ax_perror("failed on fcntl before: %s", strerror(errno));
			return (-1);
		}

		opts |= O_NONBLOCK;
		if (fcntl(fd, F_SETFL, opts) < 0) {
			ax_perror("failed on fcntl after: %s", strerror(errno));
			return (-1);
		}

		return (0);
	#endif
}

