#include "signal.h"

#include "ax/event/util.h"
#include "ax/event/event.h"
#include "ax/event/reactor.h"

#include "ax/detect.h"

#ifdef AX_OS_WIN32
	#include <windows.h>
	#include <winsock2.h>
	#define __cdecl
#else
	#include <sys/select.h>
	#include <unistd.h>
#endif

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <memory.h>
#include <signal.h>

/*
* The reactor that is listening for signal events.
*/
static ax_reactor * current_reactor;

void sig_handler(int sig){
	assert(current_reactor != NULL);
	if(current_reactor == NULL){
		ax_perror("current_reactor is null!!");
		return;
	}


	while(write(current_reactor->sig_pipe[1], &sig, 1) != 1);
}


struct signal_internal * signal_internal_init(ax_reactor * r){
	struct signal_internal *ret ;
	assert(r != NULL);
	
	if(current_reactor && current_reactor != r){
		ax_perror("Only one reactor can handle signal events.");
		return NULL;
	}

	if((ret = malloc(sizeof(struct signal_internal))) == NULL){
		ax_perror("failed to malloc for signal_internal.");
		return NULL;
	}

	memset(ret, 0, sizeof(struct signal_internal));

	current_reactor = r;
	return ret;
}


int signal_internal_register(ax_reactor * r, int sig, ax_event * e){
	struct signal_internal * psi;

	assert(r != NULL && e != NULL);
	if(current_reactor == NULL){
		ax_perror("The current_reactor hasn't been set.");
		return (-1);
	}else if(current_reactor != r){
		ax_perror("Only one reactor can handle signal events.");
		return (-1);
	}else if(r->psi == NULL){
		ax_perror("The signal_internal hasn's been set.");
		return (-1);
	}else if(e == NULL){
		ax_perror("event can not be null");
		return (-1);
	}else if(sig < 1 || sig >= SIGNALN){
		ax_perror("signal num[%d] is out of range[1-64].");
		return (-1);
	}

	psi = r->psi;

	struct sigaction setup_action;
	sigset_t block_mask;

	/* block all other signal except for the one being registered */
	sigfillset(&block_mask);
	sigdelset(&block_mask, sig);
	setup_action.sa_handler = sig_handler;
	setup_action.sa_mask = block_mask;
	setup_action.sa_flags = 0;

	if(sigaction(sig, &setup_action, &psi->old_actions[sig]) == -1){
		ax_perror("sigaction failed: %s", strerror(errno));
		return (-1);
	}

	psi->sigevents[sig] = e;

	return (0);
}

int signal_internal_restore_all(ax_reactor * r){
	struct signal_internal * psi;
	int i;

	assert(r != NULL);
	if(current_reactor == NULL){
		ax_perror("The current_reactor hasn't been set.");
		return (-1);
	}else if(current_reactor != r){
		ax_perror("Only one reactor can handle signal events.");
		return (-1);
	}else if(r->psi == NULL){
		ax_perror("The signal_internal hasn's been set.");
		return (-1);
	}

	psi = r->psi;

	for(i = 1; i < SIGNALN; ++i){
		if(psi->sigevents[i] && sigaction(i, &psi->old_actions[i], NULL) == -1){
			ax_perror("sigaction failed: %s", strerror(errno));
		}
		psi->sigevents[i] = NULL;
	}

	return (0);
}

int signal_internal_unregister(ax_reactor * r, int sig){
	struct signal_internal * psi;

	assert(r != NULL);
	if(current_reactor == NULL){
		ax_perror("The current_reactor hasn't been set.");
		return (-1);
	}else if(current_reactor != r){
		ax_perror("Only one reactor can handle signal events.");
		return (-1);
	}else if(r->psi == NULL){
		ax_perror("The signal_internal hasn's been set.");
		return (-1);
	}else if(sig < 1 || sig >= SIGNALN){
		ax_perror("signal num[%d] is out of range(1-64).");
		return (-1);
	}
	psi = r->psi;

	if (psi->sigevents[sig] == NULL) {
		ax_perror("signal num[%d] is not registered on this reactor.");
		return (-1);
	}

	if(sigaction(sig, &psi->old_actions[sig], NULL) == -1){
		ax_perror("sigaction failed: %s", strerror(errno));
		return (-1);
	}

	psi->sigevents[sig] = NULL;

	return (0);
}
