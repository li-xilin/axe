#ifndef POLLING_H
#define POLLING_H
#include "ax/event/reactor.h"

void * polling_init(ax_reactor * r);
int polling_add(ax_reactor * r, ax_socket fd, short flags);
void polling_del(ax_reactor * r, ax_socket fd, short flags);
int polling_mod(ax_reactor * r, ax_socket fd, short flags);
int polling_poll(ax_reactor * r, struct timeval * timeout);
void polling_destroy(ax_reactor * r);
void polling_print(ax_reactor * r); 

#endif
