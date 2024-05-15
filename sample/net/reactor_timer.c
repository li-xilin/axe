#include "ax/detect.h"
#include "ax/reactor.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

ax_reactor *g_reactor;

void timer_cb(ax_socket fd, short fl, void *arg)
{
	switch (*(int *)arg) {
		case 0: fputs("-", stderr); break;
		case 1: fputs("|", stderr); break;
		case 2: fputs("\n", stderr); break;
	}
	fflush(stderr);
}

int main(int argc, char *argv[]) {
	ax_socket_init();

	g_reactor = ax_reactor_create();

	ax_event timer_ev[3];
	ax_event_set(timer_ev + 0, 10, AX_EV_TIMEOUT, timer_cb, ax_p(int, 0));
	ax_event_set(timer_ev + 1, 200, AX_EV_TIMEOUT, timer_cb, ax_p(int, 1));
	ax_event_set(timer_ev + 2, 1000, AX_EV_TIMEOUT, timer_cb, ax_p(int, 2));

	ax_reactor_add(g_reactor, timer_ev + 0);
	ax_reactor_add(g_reactor, timer_ev + 1);
	ax_reactor_add(g_reactor, timer_ev + 2);

	ax_reactor_loop(g_reactor, NULL, 0);

	ax_reactor_destroy(g_reactor);
	ax_socket_exit();
	return 0;
}

