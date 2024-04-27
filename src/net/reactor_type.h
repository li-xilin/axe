/*
 * Copyright (c) 2022-2023 Li xilin <lixilin@gmx.com>
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

#ifndef REACTOR_TYPE_H
#define REACTOR_TYPE_H

#include "ax/link.h"
#include "ax/lock.h"
#include "ax/socket.h"
#include "ax/event.h"

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

#define E_OUT_OF_TIMERHEAP 0

struct timerheap_st;
struct ax_event_ht_st;

struct ax_reactor_st
{
	ax_link event_list;

	ax_link pending_list;

	struct mux_st *mux;

	struct ax_event_ht_st *eht;

	ax_lock lock;

	ax_socket io_pipe[2];

	ax_event io_event;

	struct timerheap_st *pti;

	ax_socket timer_pipe[2];

	ax_event *timer_event;

	/* Indicates whether we should stop polling. */
	int out;
};

#endif
