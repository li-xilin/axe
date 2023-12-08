/*
 * Copyright (c) 2022-2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to one person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef AX_SOCKET_H
#define	AX_SOCKET_H
#include "log.h"
#include "detect.h"
#include "trait.h"
#include <stddef.h>

#ifdef AX_OS_WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
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
#include<endian.h>
typedef int ax_socket;
#define AX_SOCKET_INVALID -1
#endif

#ifdef AX_OS_WIN32
#define ax_socket_errno() WSAGetLastError()
#define ax_socket_set_errno(errcode) WSASetLastError(errcode)
#define AX_SOCKET_ERR(name) WSA##name
#else
#define ax_socket_errno() errno
#define ax_socket_set_errno(errcode) (void)(errno = (errcode))
#define AX_SOCKET_ERR(name) name
#endif

struct timeval;

extern const ax_trait ax_t_socket;

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

inline static int64_t ax_htonll(int64_t num)
{
#ifdef AX_OS_WIN32
	return (((int64_t)htonl(num)) << 32) + htonl(num >> 32);
#else
	return (int64_t)htobe64((int64_t)num);
#endif
}

inline static int64_t ax_ntohll(int64_t num)
{
#ifdef AX_OS_WIN32
	return (((int64_t)ntohl(num)) << 32) + ntohl(num >> 32);
#else
	return (int64_t)be64toh((int64_t)num);
#endif
}

int ax_socket_set_keepalive(ax_socket sock, uint32_t idle_sec, uint32_t interval_sec);

#endif
