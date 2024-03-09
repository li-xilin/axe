/*
 * Copyright (c) 2023-2024 Li Xilin <lixilin@gmx.com>
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

#ifndef AX_IOBUF_H
#define AX_IOBUF_H

#include "ax/def.h"
#include "ax/mem.h"
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

struct ax_iobuf_st
{
	size_t size, front, rear;
	uint8_t *buf;
};

typedef struct ax_iobuf_st ax_iobuf;
typedef void ax_iobuf_drain_cb(void *data, size_t size, void *arg);

inline static void ax_iobuf_init(ax_iobuf *b, void *buf, size_t size)
{
	assert(size > 1);
	b->size = size;
	b->buf = buf;
	b->rear = b->front = 0;
}

inline static size_t ax_iobuf_data_size(ax_iobuf *b)
{
	return (b->size + b->rear - b->front) % b->size;
}

inline static size_t ax_iobuf_full(ax_iobuf *b)
{
	return (b->rear + 1) % b->size == b->front;
}

inline static size_t ax_iobuf_empty(ax_iobuf *b)
{
	return b->rear == b->front;
}

inline static size_t ax_iobuf_max_size(ax_iobuf *b)
{
	return b->size - 1;
}

inline static size_t ax_iobuf_free_size(ax_iobuf *b)
{
	return ax_iobuf_max_size(b) - ax_iobuf_data_size(b);
}

inline static void ax_iobuf_clear(ax_iobuf *b)
{
	b->rear = b->front = 0;
}

size_t ax_iobuf_write(ax_iobuf *b, void *p, size_t size);

size_t ax_iobuf_peek(ax_iobuf *b, void *buf, size_t start, size_t size);

size_t ax_iobuf_read(ax_iobuf *b, void *buf, size_t size);
void *ax_iobuf_chbuf(ax_iobuf *b, void *buf, size_t size);

size_t ax_iobuf_drain(ax_iobuf *b, size_t size, ax_iobuf_drain_cb *cb, void *arg);

void *ax_iobuf_flatten(ax_iobuf *b);

#endif
