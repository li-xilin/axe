/*
* Copyright (c) 2014 Xinjing Chow
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
* 3. The name of the author may not be used to endorse or promote products
*    derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
* IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
* OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
* IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
* NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
* DATA, OR PROFITS; OR BUSINESS INTERR
*/
#ifndef AX_EVENT_SKUTIL_H
#define	AX_EVENT_SKUTIL_H
#include "../log.h"
#include "../detect.h"

#ifdef AX_OS_WIN32
#include "winsock.h"
#define ax_socket intptr_t
#else
#include "sys/time.h"
#define ax_socket int
#endif

struct timeval;

#define ax_skutil_timesub(lv, rv, ov) do{\
		(ov)->tv_sec = (lv)->tv_sec - (rv)->tv_sec;\
		(ov)->tv_usec = (lv)->tv_usec - (rv)->tv_usec;\
		if((ov)->tv_usec < 0) {\
			(ov)->tv_usec += 1000000;\
			(ov)->tv_sec--;\
		}\
	} while(0);

/*
* Create a pair of connected sockets
* Return: 0 on success, -1 on failure
* @pipe: used to return the newly created sockets
*/
int ax_skutil_create_pipe(ax_socket pipe[2]);

/*
* Close a socket.
* Return: 0 on success, -1 on failure
* @fd: the socket file descriptor to close
*/
int ax_skutil_close_fd(ax_socket fd);

/*
* Get the system time and store it in the tv.
* Return: -1 on failure, 0 on success
* @tv: the timeval structure in which the time where be store
*/
int ax_skutil_gettimeofday(struct timeval * tv);

/*
* Set the fd to nonblocking mode.
* Return: -1 on failure, 0 on success.
* @fd: the fd to set.
*/
int ax_skutil_set_nonblocking(ax_socket fd);

#endif
