#ifndef AX_EVENT_UTIL_H
#define	AX_EVENT_UTIL_H
#include "log.h"
#include "detect.h"
#include "trait.h"
#include <stddef.h>

#ifdef AX_OS_WIN32
#include <winsock2.h>
typedef SOCKET ax_socket;
#else
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
typedef int ax_socket;
#endif

struct timeval;

extern const ax_trait ax_trait_type_ax_socket;

int ax_socket_init();

void ax_socket_deinit();

int ax_socket_pair(int family, int type, int protocol, ax_socket fd[2]);

int ax_socket_close(ax_socket fd);

int ax_socket_set_nonblocking(ax_socket fd);

#endif
