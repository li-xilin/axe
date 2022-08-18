#include "ax/event/reactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#ifdef AX_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

void read_cb(ax_socket fd, short fl, void *arg)
{
	char buf[1024];
	int len = read(fd, buf, sizeof buf - 1);
	if (len == 0) {
		ax_util_close_fd(fd);
		printf("client lost: %" PRIu64 "\n", (uint64_t)fd);
	} else {
		write(fd, buf, len);
	}

}

void listen_cb(ax_socket fd, short fl, void *arg)
{
	struct sockaddr_in sa;
	socklen_t len;
	int cli_fd = accept(fd, (struct sockaddr *)&sa, &len);

	printf("client found: %" PRIu64 ", %s:%hu\n", (uint64_t)cli_fd, inet_ntoa(sa.sin_addr), ntohs(sa.sin_port));

	ax_event *ev = malloc(sizeof *ev);
	ax_event_set(ev, cli_fd, E_READ, read_cb, arg);

	ax_reactor_add(arg, ev);
}

int main(int argc, char *argv[]) {
	if (argc < 2) {
		puts("Usage: reactor_echo <port>");
		exit(EXIT_FAILURE);
	}
	uint16_t port = atoi(argv[1]);;

	ax_reactor reactor;
	ax_reactor_init(&reactor);

	int fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (fd == -1) {
		perror("socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sa;
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = INADDR_ANY;
	bind(fd, (struct sockaddr *)&sa, sizeof sa);
	listen(fd, 1024);

	printf("listen to %hu\n", port);

	ax_event read_ev;
	ax_event_set(&read_ev, fd, E_READ, listen_cb, &reactor);
	ax_reactor_add(&reactor, &read_ev);

	ax_reactor_loop(&reactor, NULL, 0);

	return 0;
}

