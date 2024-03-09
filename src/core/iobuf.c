/*
 * Copyright (c) 2024 Li Xilin <lixilin@gmx.com>
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

#include "ax/iobuf.h"
#include "check.h"

size_t ax_iobuf_write(ax_iobuf *b, void *p, size_t size)
{
        assert(p != NULL);
        size_t buf_size = ax_min(ax_iobuf_free_size(b), size);
        size_t size1 = ax_min(b->size - b->rear, buf_size);
        memcpy(b->buf + b->rear, p, size1);
        memcpy(b->buf, (uint8_t *)p + size1, buf_size - size1);
        b->rear = (b->rear + buf_size) % b->size;
        return buf_size;
}

size_t ax_iobuf_peek(ax_iobuf *b, void *buf, size_t start, size_t size)
{
        assert(buf != NULL);

        size_t data_size = ax_iobuf_data_size(b);
        if (start >= data_size)
                return 0;

        size_t read_size = ax_min(ax_iobuf_data_size(b) - start, size);
        size_t new_front = (b->front + start) % b->size;
        size_t size1 = ax_min(b->size - new_front, read_size);

        memcpy(buf, b->buf + new_front, size1);
        memcpy((uint8_t *)buf + size1, b->buf, read_size - size1);
        return read_size;
}

size_t ax_iobuf_read(ax_iobuf *b, void *buf, size_t size)
{
        size_t read_size = ax_min(ax_iobuf_data_size(b), size);
        if (buf) {
                size_t size1 = ax_min((b->size - b->front), read_size);
                memcpy(buf, b->buf + b->front, size1);
                memcpy((uint8_t *)buf + size1, b->buf, read_size - size1);
        }
        b->front = (b->front + read_size) % b->size;
        return read_size;
}

void *ax_iobuf_chbuf(ax_iobuf *b, void *buf, size_t size)
{
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(buf);
	CHECK_PARAM_VALIDITY(size, size > 1);

        if (ax_iobuf_data_size(b) < size - 1) {
                errno = EINVAL;
                return NULL;
        }

        if (b->front <= b->rear)
                memcpy(buf, b->buf + b->front, b->rear - b->front);
        else {
                memcpy(buf, b->buf + b->front, b->size - b->front);
                memcpy((ax_byte *)buf + b->size - b->front, b->buf , b->rear);
        }

        b->buf = buf;
        b->rear = ax_iobuf_data_size(b);
        b->front = 0;
        return b->buf;
}

size_t ax_iobuf_drain(ax_iobuf *b, size_t size, ax_iobuf_drain_cb *cb, void *arg)
{
	CHECK_PARAM_NULL(b);
	CHECK_PARAM_NULL(cb);

        size_t read_size = ax_min(ax_iobuf_data_size(b), size);
        size_t size1 = ax_min((b->size - b->front), read_size);
        if (size1)
                cb(b->buf + b->front, size1, arg);
        if (read_size - size1)
                cb(b->buf, read_size - size1, arg);
        b->front = (b->front + read_size) % b->size;
        return read_size;
}

void *ax_iobuf_flatten(ax_iobuf *b)
{
	CHECK_PARAM_NULL(b);

        if (b->front <= b->rear)
                return b->buf + b->front;

        size_t size = (b->rear + b->size - b->front) % b->size;
        size_t left = 0, mid = b->rear, right = b->front, d = 0;

        while (1) {
                if (mid - left < b->size - right) {
                        size_t swp_len = mid - left;
                        if (d % 2 == 1)
                                memcpy(b->buf + left, b->buf + right, swp_len);
                        else
                                ax_memswp(b->buf + left, b->buf + right, swp_len);
                        left += swp_len;
                        mid = right;
                        right = mid + swp_len;
                        d += 1;
                }
                else {
                        size_t swp_len = b->size - right;
                        ax_memswp(b->buf + left, b->buf + right, swp_len);
                        left += swp_len;
                        if (d % 2 == 1)
                                break;
                        d = 0;
                }

        }
        memcpy(b->buf + left, b->buf + mid, right - mid);
        b->front = 0;
        b->rear = size;
        return b->buf + b->front;
}
