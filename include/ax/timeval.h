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

#ifndef AX_TIMEVAL_H
#define AX_TIMEVAL_H

#include "ax/detect.h"

#ifdef AX_OS_WIN32
#include <sysinfoapi.h>
#else
#include <sys/time.h>
#endif

inline static int ax_timeval_timeofday(struct timeval * tv)
{
#ifdef AX_OS_WIN32
	union {
		long long ns100;
		FILETIME ft;
	} now;
	GetSystemTimeAsFileTime (&now.ft);
	tv->tv_usec = (long) ((now.ns100 / 10LL) % 1000000LL);
	tv->tv_sec = (long) ((now.ns100 - 116444736000000000LL) / 10000000LL);
	return (0);
#else
	return gettimeofday(tv, NULL);
#endif
}

#define ax_timeval_sub(tv1, tv2, tv_out) \
	do { \
		(tv_out)->tv_sec = (tv1)->tv_sec - (tv2)->tv_sec; \
		(tv_out)->tv_usec = (tv1)->tv_usec - (tv2)->tv_usec; \
		if ((tv_out)->tv_usec < 0) { \
			(tv_out)->tv_usec += 1000000; \
			(tv_out)->tv_sec--; \
		} \
	} while(0);

#define ax_timeval_add_millise(tv, millise) \
	do { \
		(tv).tv_sec += (millise) / 1000; \
		(tv).tv_usec += ((millise) % 1000) * 1000; \
		if ((tv).tv_usec > 1000000){ \
			(tv).tv_usec -= 1000000; \
			(tv).tv_sec++; \
		} \
	} while(0);

#define ax_timeval_sub_millise(tv, millise) \
	do { \
		(tv).tv_sec -= (millise) / 1000; \
		(tv).tv_usec -= ((millise) % 1000) * 1000; \
		if ((tv).tv_usec < 0){ \
			(tv).tv_usec += 1000000; \
			(tv).tv_sec--; \
		} \
	} while(0);

#define ax_timeval_from_millise(tv, millise) \
	do { \
		(tv).tv_sec = (millise) / 1000; \
		(tv).tv_usec = ((millise) % 1000) * 1000; \
	} while(0);	
#define ax_timeval_to_millise(tv) \
	((tv).tv_sec * 1000 + (tv).tv_usec / 1000)

#define ax_timeval_ge(t1, t2) \
	((t1).tv_sec > (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec >= (t2).tv_usec))

#define ax_timeval_le(t1, t2) \
	((t1).tv_sec < (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec <= (t2).tv_usec))

#define ax_timeval_g(t1, t2) \
	((t1).tv_sec > (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec > (t2).tv_usec))

#define ax_timeval_l(t1, t2) \
	((t1).tv_sec < (t2).tv_sec || ((t1).tv_sec == (t2).tv_sec && (t1).tv_usec < (t2).tv_usec))

#endif

