#ifndef AX_EVENT_UTIL_H
#define	AX_EVENT_UTIL_H
#include "log.h"
#include "detect.h"
#include "trait.h"
#include <stddef.h>

#ifdef AX_OS_WIN32
#include <winsock2.h>
typedef SOCKET ax_socket;
#define AX_SOCKET_INVALID INVALID_SOCKET
#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif
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


int ax_socket_waitrecv(ax_socket sock, size_t millise);


inline static int ax_socket_recv_u16(ax_socket sock, uint16_t *valuep)
{
        uint16_t value;
        if (ax_socket_syncrecv(sock, &value, sizeof value))
                return -1;
        *valuep = ntohs(value);
        return 0;
}

inline static int ax_socket_recv_u32(ax_socket sock, uint32_t *valuep)
{
        uint32_t value;
        if (ax_socket_syncrecv(sock, &value, sizeof value))
                return -1;
        *valuep = ntohl(value);
        return 0;
}

inline static int ax_socket_send_u16(ax_socket sock, uint16_t value)
{
        uint16_t value_n = htons(value);
        if (ax_socket_syncsend(sock, &value_n, sizeof value_n))
                return -1;
        return 0;
}

inline static int ax_socket_send_u32(ax_socket sock, uint32_t value)
{
        uint32_t value_n = htonl(value);
        if (ax_socket_syncsend(sock, &value_n, sizeof value_n))
                return -1;
        return 0;
}

#endif
