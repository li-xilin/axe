#ifndef AX_EVENT_UTIL_H
#define	AX_EVENT_UTIL_H
#include "log.h"
#include "detect.h"
#include "trait.h"
#include <stddef.h>

#ifdef AX_OS_WIN32
#include <winsock2.h>
typedef SOCKET ax_socket;
#define AX_SOCKET_INVALID SOCKET_INVALID
#else
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
typedef int ax_socket;
#define AX_SOCKET_INVALID -1
#endif

struct timeval;

ax_trait_declare(ax_socket, ax_socket);

int ax_socket_init();

void ax_socket_deinit();

int ax_socket_pair(int family, int type, int protocol, ax_socket fd[2]);

int ax_socket_close(ax_socket fd);

int ax_socket_set_nonblocking(ax_socket fd);

int ax_socket_syncsend(ax_socket sock, const void *data, size_t len);
int ax_socket_syncrecv(ax_socket sock, void *buf, size_t len);

#endif
