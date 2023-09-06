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

#ifndef AX_REACTOR_H
#define AX_REACTOR_H

#include "socket.h"
#include "event.h"

#ifndef AX_REACTOR_DEFINED
#define AX_REACTOR_DEFINED
typedef struct ax_reactor_st ax_reactor;
#endif

/* exits after having processed at least one active event */
#define AX_REACTOR_ONCE 0x01

ax_reactor *ax_reactor_create();

void ax_reactor_exit(ax_reactor *r);

void ax_reactor_destroy(ax_reactor *r);

int ax_reactor_add(ax_reactor *r, ax_event *e);

int ax_reactor_modify(ax_reactor *r, ax_event *e);

void ax_reactor_add_to_pending(ax_reactor *r, ax_event *e, short res_flags);

int ax_reactor_remove(ax_reactor *r, ax_event *e);

bool ax_reactor_empty(ax_reactor *r);

void ax_reactor_clear(ax_reactor *r);

void ax_reactor_loop(ax_reactor *r, struct timeval *timeout, int flags);

#endif
