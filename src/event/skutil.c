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
#include "ax/event/skutil.h"
#include "ax/log.h"

#ifdef AX_OS_WIN32
#include <windows.h>
#else
#include <unistd.h>
#include <fcntl.h>
#include <sys/time.h>
#endif

#include <errno.h>
#include <string.h>

inline int el_create_pipe(ax_socket p[2]){
	#ifdef WIN32
		return CreatePipe(&p[0], &p[1], NULL, 0) ? 0 : -1;
	#else
		return pipe(p);
	#endif
}
inline int el_close_fd(ax_socket fd){
	#ifdef WIN32
		return closesocket(fd) ? -1 : 0;
	#else
		return close(fd);
	#endif
}

inline int el_gettimeofday(struct timeval * tv){
	#ifdef WIN32
		union{
			long long ns100;
			FILETIME ft;
		}u_time;
		GetSystemTimeAsFileTime (&u_time.ft);
	    tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	    tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
	    return (0);
	#else
	    return gettimeofday(tv, NULL);
	#endif
}

inline int el_set_nonblocking(ax_socket fd){
	#ifdef WIN32
		ULONG nonBlock = 1;
		if (ioctlsocket(m_Socket, FIONBIO, &nonBlock)){
			ax_perror("failed on ioctlsocket");
			return (-1);
		}
		return (0);
	#else
		int opts;

		if((opts = fcntl(fd, F_GETFL)) < 0){
			ax_perror("failed on fcntl before: %s", strerror(errno));
			return (-1);
		}

		opts |= O_NONBLOCK;
		if(fcntl(fd, F_SETFL, opts) < 0){
			ax_perror("failed on fcntl after: %s", strerror(errno));
			return (-1);
		}

		return (0);
	#endif
}
