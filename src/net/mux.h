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

#ifndef NET_MUX_H
#define NET_MUX_H

#include "event_ht.h"
#include "ax/reactor.h"
#include "ax/lock.h"

typedef void mux_pending_cb(ax_socket fd, short flags, void *arg);

typedef struct mux_st mux;

mux *mux_init(void);
int mux_add(mux *mux, ax_socket fd, short flags);
int mux_mod(mux *mux, ax_socket fd, short flags);
void mux_del(mux *mux, ax_socket fd, short flags);
int mux_poll(mux *mux, ax_lock *lock, struct timeval * timeout, mux_pending_cb *cb, void *arg);
void mux_free(mux *mux);

#endif
