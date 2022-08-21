#include "ax/detect.h"

#ifdef AX_OS_WIN32
#include <ws2tcpip.h>
#include <winsock.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

#include "ax/event/reactor.h"

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#ifdef AX_OS_WIN32
#define last_error() (int)GetLastError()
#else
#define last_error() (int)errno
#endif

ax_reactor g_reactor;

void read_cb(ax_socket fd, short fl, void *arg)
{
	char buf[1024];
	int len = recv(fd, buf, sizeof buf - 1, 0);
	if (len == 0) {
		ax_reactor_remove(&g_reactor, arg);
		ax_util_close_fd(fd);
		printf("client lost: %" PRIu64 "\n", (uint64_t)fd);
	} else {
		send(fd, buf, len, 0);
	}

}

void listen_cb(ax_socket fd, short fl, void *arg)
{
	struct sockaddr_in sa;
	socklen_t len;
	int cli_fd = accept(fd, (struct sockaddr *)&sa, &len);

	printf("client found: %" PRIu64 ", %s:%hu\n", (uint64_t)cli_fd, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));

	ax_event *ev = malloc(sizeof *ev);
	ax_event_set(ev, cli_fd, E_READ, read_cb, ev);

	ax_reactor_add(arg, ev);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		puts("Usage: reactor_echo <port>");
		exit(EXIT_FAILURE);
	}

#ifdef AX_OS_WIN32
	WORD wVersionRequested;
	WSADATA wsaData;
	wVersionRequested=MAKEWORD(1,1);
	WSAStartup(wVersionRequested,&wsaData);
#endif
	uint16_t port = atoi(argv[1]);;

	ax_reactor_init(&g_reactor);

	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	int ret = -1;

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = INADDR_ANY;
	ret = bind(fd, (struct sockaddr *)&sa, sizeof sa);
	if (ret < 0) {
		printf("bind %d\n", last_error());
		exit(EXIT_FAILURE);
	}
	ret = listen(fd, 1024);
	if (ret < 0) {
		printf("bind %d\n", last_error());
		exit(EXIT_FAILURE);
	}

	printf("listen to %hu\n", port);

	ax_event read_ev;
	ax_event_set(&read_ev, fd, E_READ, listen_cb, &g_reactor);
	ax_reactor_add(&g_reactor, &read_ev);

	ax_reactor_loop(&g_reactor, NULL, 0);

	return 0;
}

