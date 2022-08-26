/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
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

#ifndef AX_QUEUE_H
#define AX_QUEUE_H
#include "type/tube.h"

#ifndef AX_QUEUE_DEFINED
#define AX_QUEUE_DEFINED
typedef struct ax_queue_st ax_queue;
#endif

#define ax_baseof_ax_queue ax_tube
ax_concrete_declare(3, ax_queue);

ax_tube *__ax_queue_construct(const ax_trait *elem_tr);

inline static ax_concrete_creator(ax_queue, const ax_trait *tr)
{
	return __ax_queue_construct(tr);
}

ax_fail ax_queue_push(ax_queue *queue, const void *);

void ax_queue_pop(ax_queue *queue, const void *);

#endif
