/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
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
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

struct ax_iobuf_st
{
        size_t size, front, rear;
        uint8_t *buf;
};

typedef struct ax_iobuf_st ax_iobuf;

inline static void ax_iobuf_init(ax_iobuf *b, void *buf, size_t size)
{
	b->size = size;
	b->buf = (uint8_t *)buf;
	b->rear = b->front = 0;
}

inline static size_t ax_iobuf_size(ax_iobuf *b)
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

inline static void ax_iobuf_clear(ax_iobuf *b)
{
	b->rear = b->front = 0;
}

inline static size_t ax_iobuf_write(ax_iobuf *b, void *p, size_t size)
{
	size_t writen_size = ax_min(ax_iobuf_max_size(b) - ax_iobuf_size(b), size);
	size_t size1 = (b->size - b->rear) % writen_size;
	memcpy(b->buf + b->rear, p, size1);
	memcpy(b->buf, (uint8_t *)p + size1, writen_size - size1);
	b->rear = (b->rear + writen_size) % b->front;
	return writen_size;
}

inline static size_t ax_iobuf_read(ax_iobuf *b, void *buf, size_t size)
{
	size_t read_size = ax_min(ax_iobuf_size(b), size);
	size_t size1 = ax_min((b->size - b->front), read_size);
	memcpy(buf, b->buf + b->front, size1);
	memcpy((uint8_t *)buf + size1, b->buf, read_size - size1);
	b->front = (b->front + read_size) % b->size;
	return read_size;
}

inline static void *ax_iobuf_chbuf(ax_iobuf *b, void *buf, size_t size)
{
	if (ax_iobuf_size(b) < size - 1) {
		errno = EINVAL;
		return NULL;
	}
	b->rear = ax_iobuf_read(b, buf, size);
	b->front = 0;
	return b->buf;
}

#endif
