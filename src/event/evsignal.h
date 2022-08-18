/*
 * Copyright (c) 2014 Xinjing Chow
 *
 */
#ifndef AX_EVENT_SIGNAL_H
#define AX_EVENT_SIGNAL_H

#include <signal.h>

#define SIGNALN 65

typedef void (* signal_handler)(int);

#ifndef AX_EVENT_DEFINED
#define AX_EVENT_DEFINED
typedef struct ax_event_st ax_event;
#endif

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

struct signal_internal{
	/* Used to restore original signal handler */
	struct sigaction old_actions[SIGNALN];

	/* 
	 * Every signal only has one registered event.
	 * The last one will take effect if there
	 * are multiple events registering to the 
	 * same signal.
	 */
	ax_event *sigevents[SIGNALN];
};

/*
 * Signal handler to inform the reactor that a signal has occured.
 * @sig: the signal that just occured.
 */
void sig_handler(int sig);

/*
 * Allocate and initialize the signal_internal structure.
 * Return: newly created signal_internal on success, NULL on failure.
 * @r: the reactor that handles signals.
 */
struct signal_internal *signal_internal_init(ax_reactor *r);

/*
 * Register a signal to the signal_internal.
 * Return: 0 on success, -1 on failure.
 * @r: the related reacotr.
 * @sig: the signal to regitser.
 * @e: the signal event.
 */
int signal_internal_register(ax_reactor *r, int sig, ax_event *e);

/*
 * Unregister a signal from the signal_internal.
 * Return: 0 on success, -1 on failure.
 * @r: the related reacotr.
 * @sig: the signal to unregitser.
 */
int signal_internal_unregister(ax_reactor *r, int sig);

int signal_internal_restore_all(ax_reactor *r);

#endif
