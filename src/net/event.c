/*
 * Copyright (c) 2022-2024 Li Xilin <lixilin@gmx.com>
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

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "reactor_type.h"
#include "ax/socket.h"
#include "ax/event.h"
#include "ax/link.h"

void ax_event_set(ax_event *e, ax_socket fd, short ev_flags, ax_event_cb_f *cb, void *arg){
	assert(e != NULL);

	e->ev_flags = ev_flags;
	e->cb = cb;
	e->arg = arg;
	e->fd = fd;
	e->timerheap_idx = E_OUT_OF_TIMERHEAP;
	ax_link_init_empty(&e->event_link);
	ax_link_init_empty(&e->pending_link);
	ax_link_init_empty(&e->hash_link);
}

bool ax_event_in_use(const ax_event *e){
	assert(e != NULL);

	return e->ev_flags & AX_EV_REACTING;
}

