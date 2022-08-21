#include "ax/event/util.h"
#include "ax/log.h"
#include "ax/mem.h"

#ifdef AX_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/socket.h>
#endif

#include <errno.h>
#include <string.h>
#include <assert.h>

bool socket_equal(const void *s1, const void *s2)
{
	return *(ax_socket *)s1 == *(ax_socket *)s2;
}

size_t socket_hash(const void *p)
{
	return ax_memhash(p, sizeof(ax_socket));
}

const ax_trait ax_trait_ax_socket = {
	.size = sizeof(ax_socket),
	.equal = socket_equal,
	.hash = socket_hash,
};

int ax_util_close_fd(ax_socket fd) {
#ifdef AX_OS_WIN32
	return closesocket(fd) ? -1 : 0;
#else
	return close(fd);
#endif
}

/*
int ax_util_pipe_read(ax_socket fd, void *buf, size_t size, size_t *real)
{
#ifdef AX_OS_WIN32
	DWORD dwRealRead;
	if (!ReadFile((HANDLE)fd, buf, size, &dwRealRead, NULL))
		return -1;
	*real = dwRealRead;
#else
	
#endif
	return 0;
}

int ax_util_pipe_write(ax_socket fd, void *data, size_t size, size_t *real)
{
#ifdef AX_OS_WIN32
	DWORD dwRealWriten;
	if (!WriteFile((HANDLE)fd, data, size, &dwRealWriten, NULL))
		return -1;
	*real = dwRealWriten;
#else
#endif
	return 0;
}
*/

int ax_util_timeofday(struct timeval * tv)
{
#ifdef AX_OS_WIN32
	union {
		long long ns100;
		FILETIME ft;
	} now;
	GetSystemTimeAsFileTime (&now.ft);
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
	if (ioctlsocket(fd, FIONBIO, &nonBlock)) {
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




#ifdef _WIN32

static int socketpair_ersatz(int family, int type, int protocol, ax_socket fd[2])
{
	assert(fd);

	ax_socket listener = -1;
	ax_socket connector = -1;
	ax_socket acceptor = -1;
	struct sockaddr_in listen_addr;
	struct sockaddr_in connect_addr;
	socklen_t size;
	int saved_errno = -1;
	int family_test;
	
	family_test = family != AF_INET;
#ifdef AF_UNIX
	family_test = family_test && (family != AF_UNIX);
#endif
	if (protocol || family_test) {
		ax_perror("family is not supported");
		return -1;
	}
	
	listener = socket(AF_INET, type, 0);
	if (listener < 0)
		return -1;
	memset(&listen_addr, 0, sizeof(listen_addr));
	listen_addr.sin_family = AF_INET;
	listen_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	listen_addr.sin_port = 0;	/* kernel chooses port.	 */
	if (bind(listener, (struct sockaddr *) &listen_addr, sizeof (listen_addr)) == -1)
		goto tidy_up_and_fail;
	if (listen(listener, 1) == -1)
		goto tidy_up_and_fail;

	connector = socket(AF_INET, type, 0);
	if (connector < 0)
		goto tidy_up_and_fail;

	memset(&connect_addr, 0, sizeof(connect_addr));

	/* We want to find out the port number to connect to.  */
	size = sizeof(connect_addr);
	if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;
	if (size != sizeof (connect_addr))
		goto abort_tidy_up_and_fail;
	if (connect(connector, (struct sockaddr *) &connect_addr,
				sizeof(connect_addr)) == -1)
		goto tidy_up_and_fail;

	size = sizeof(listen_addr);
	acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
	if (acceptor < 0)
		goto tidy_up_and_fail;
	if (size != sizeof(listen_addr))
		goto abort_tidy_up_and_fail;
	/* Now check we are talking to ourself by matching port and host on the
	   two sockets.	 */
	if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;
	if (size != sizeof (connect_addr)
		|| listen_addr.sin_family != connect_addr.sin_family
		|| listen_addr.sin_addr.s_addr != connect_addr.sin_addr.s_addr
		|| listen_addr.sin_port != connect_addr.sin_port)
		goto abort_tidy_up_and_fail;
	ax_util_close_fd(listener);
	fd[0] = connector;
	fd[1] = acceptor;

	return 0;

 abort_tidy_up_and_fail:
	saved_errno = WSAECONNABORTED;
 tidy_up_and_fail:
	if (saved_errno < 0)
		saved_errno = WSAGetLastError();
	if (listener != -1)
		ax_util_close_fd(listener);
	if (connector != -1)
		ax_util_close_fd(connector);
	if (acceptor != -1)
		ax_util_close_fd(acceptor);

	WSASetLastError(saved_errno);
	return -1;
}

#ifdef HAVE_AFUNIX_H
static int create_tmpfile(char tmpfile[MAX_PATH])
{
	char short_path[MAX_PATH] = {0};
	char long_path[MAX_PATH] = {0};
	char prefix[4] = "axe";

	/*
	const char *base32set = "abcdefghijklmnopqrstuvwxyz012345";
	prefix[0] = base32set[rand() % 32];
	prefix[1] = base32set[rand() % 32];
	prefix[2] = base32set[rand() % 32];
	prefix[3] = '\0';
	*/

	GetTempPathA(MAX_PATH, short_path);
	GetLongPathNameA(short_path, long_path, MAX_PATH);
	if (!GetTempFileNameA(long_path, prefix, 0, tmpfile)) {
		ax_pwarn("GetTempFileName failed: %d", GetLastError());
		return -1;
	}
	return 0;
}

/* Test whether Unix domain socket works.
 * Return 1 if it works, otherwise 0 		*/
static int check_working_afunix()
{
	/* Windows 10 began to support Unix domain socket. Let's just try
	 * socket(AF_UNIX, , ) and fall back to socket(AF_INET, , ).
	 * https://devblogs.microsoft.com/commandline/af_unix-comes-to-windows/
	 */
	ax_socket sd = -1;
	if (have_working_afunix_ < 0) {
		if ((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
			have_working_afunix_ = 0;
		}
		else {
			have_working_afunix_ = 1;
			ax_util_close_fd(sd);
		}
	}
	return have_working_afunix_;
}

static int socketpair_win32_afunix(int family, int type, int protocol, ax_socket fd[2])
{
	assert(fd);

	ax_socket listener = -1;
	ax_socket connector = -1;
	ax_socket acceptor = -1;

	struct sockaddr_un listen_addr;
	struct sockaddr_un connect_addr;
	char tmp_file[MAX_PATH] = {0};

	ev_socklen_t size;
	int saved_errno = -1;

	listener = socket(family, type, 0);
	if (listener < 0)
		return -1;

	memset(&listen_addr, 0, sizeof(listen_addr));

	if (create_tmpfile(tmp_file)) {
		goto tidy_up_and_fail;
	}
	DeleteFileA(tmp_file);
	listen_addr.sun_family = AF_UNIX;
	if (strlcpy(listen_addr.sun_path, tmp_file, UNIX_PATH_MAX) >=
		UNIX_PATH_MAX) {
		event_warnx("Temp file name is too long");
		goto tidy_up_and_fail;
	}

	if (bind(listener, (struct sockaddr *) &listen_addr, sizeof (listen_addr))
		== -1)
		goto tidy_up_and_fail;
	if (listen(listener, 1) == -1)
		goto tidy_up_and_fail;

	connector = socket(family, type, 0);
	if (connector < 0)
		goto tidy_up_and_fail;

	memset(&connect_addr, 0, sizeof(connect_addr));

	/* We want to find out the port number to connect to.  */
	size = sizeof(connect_addr);
	if (getsockname(listener, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;

	if (connect(connector, (struct sockaddr *) &connect_addr,
				sizeof(connect_addr)) == -1)
		goto tidy_up_and_fail;

	size = sizeof(listen_addr);
	acceptor = accept(listener, (struct sockaddr *) &listen_addr, &size);
	if (acceptor < 0)
		goto tidy_up_and_fail;
	if (size != sizeof(listen_addr))
		goto abort_tidy_up_and_fail;
	/* Now check we are talking to ourself by matching port and host on the
	   two sockets.	 */
	if (getsockname(connector, (struct sockaddr *) &connect_addr, &size) == -1)
		goto tidy_up_and_fail;

	if (size != sizeof(connect_addr) ||
	    listen_addr.sun_family != connect_addr.sun_family || _stricmp(listen_addr.sun_path, connect_addr.sun_path))
		goto abort_tidy_up_and_fail;

	ax_util_close_fd(listener);
	fd[0] = connector;
	fd[1] = acceptor;

	return 0;

 abort_tidy_up_and_fail:
	saved_errno = ERR(ECONNABORTED);
 tidy_up_and_fail:
	if (saved_errno < 0)
		saved_errno = EVUTIL_SOCKET_ERROR();
	if (listener != -1)
		ax_util_close_fd(listener);
	if (connector != -1)
		ax_util_close_fd(connector);
	if (acceptor != -1)
		ax_util_close_fd(acceptor);
	if (tmp_file[0])
		DeleteFileA(tmp_file);

	EVUTIL_SET_SOCKET_ERROR(saved_errno);
	return -1;
}
#endif

static int socketpair_win32(int family, int type, int protocol, ax_socket fd[2])
{
#ifdef HAVE_AFUNIX_H
	if (protocol || (family != AF_UNIX && family != AF_INET))
		return -1;

	if (check_working_afunix()) {
		family = AF_UNIX;
		if (type != SOCK_STREAM) {
			/* Win10 does not support AF_UNIX socket of a type other
			 * than SOCK_STREAM still now. */
			family = AF_INET;
		}
	} else {
		family = AF_INET;
	}
	if (family == AF_UNIX)
		return socketpair_win32_afunix(family, type, protocol, fd);

#endif
	return socketpair_ersatz(family, type, protocol, fd);
}
#endif

int ax_util_socketpair(int family, int type, int protocol, ax_socket fd[2])
{
#ifndef _WIN32
	return socketpair(family, type, protocol, fd);
#else
	return socketpair_win32(family, type, protocol, fd);
#endif
}

