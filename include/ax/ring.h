/*
 * Copyright (c) 2020-2023 Li Xilin <lixilin@gmx.com>
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

#include "ax/def.h"
#include "ax/trick.h"
#include "ax/debug.h"
#include <stdlib.h>
#include <string.h>


#ifndef TYPE
#error "TYPE macro not defined"
#endif

#ifndef NAME
#error "NAME macro not defined"
#endif

#define AX_RING(tail) AX_CATENATE(NAME, ring_, tail)
#define _AX_RING(tail) AX_CATENATE(_, NAME, ring_, tail)
#define ax_ring_st AX_RING(st)
#define ax_ring_init AX_RING(init)
#define ax_ring_free AX_RING(free)
#define ax_ring_size AX_RING(size)
#define ax_ring_push_back AX_RING(push_back)
#define ax_ring_push_front AX_RING(push_front)
#define ax_ring_pop_back AX_RING(pop_back)
#define ax_ring_pop_front AX_RING(pop_front)
#define ax_ring_front AX_RING(front)
#define ax_ring_back AX_RING(back)
#define ax_ring_at AX_RING(at)
#define ax_ring_clear AX_RING(clear)
#define ax_ring_offset AX_RING(offset)
#define ax_ring_at_offset AX_RING(at_offset)
#define ax_ring_at_offset AX_RING(at_offset)
#define ax_ring_offset_to_index AX_RING(offset_to_index)

#define _ax_ring_full(ring) ((ring->rear + 1) % ring->size == ring->front)
#define _ax_ring_empty(ring) (ring->front == ring->rear)
#define _ax_ring_resize _AX_RING(resize)

struct ax_ring_st
{
	TYPE *buf;
	size_t size;
	size_t front, rear;
};

inline static size_t ax_ring_size(const struct ax_ring_st *ring);

static bool ax_ring_offset_is_valid(const struct ax_ring_st *ring, size_t offset)
{
	return offset >= ring->front
		? ring->rear > offset || ring->rear < ring->front
		: ring->rear > offset;
}

static void _ax_ring_resize(struct ax_ring_st *ring, TYPE *new_buf, size_t new_size)
{
	if (ring->rear >= ring->front) {
		memcpy(new_buf, ring->buf + ring->front, (ring->rear - ring->front) * sizeof(TYPE));
		ring->rear = ring->rear - ring->front;
	} else {
		memcpy(new_buf, ring->buf + ring->front, (ring->size - ring->front) * sizeof(TYPE));
		memcpy(new_buf + (ring->size - ring->front), ring->buf, ring->rear * sizeof(TYPE));
		ring->rear = ring->size - ring->front + ring->rear;
	}
	free(ring->buf);
	ring->front = 0;
	ring->size = new_size;
	ring->buf = new_buf;
}

static void ax_ring_free(struct ax_ring_st *ring)
{
	if (!ring)
		return;
	free(ring->buf);
}

static ax_fail ax_ring_push_front(struct ax_ring_st *ring, TYPE *value)
{
	if (_ax_ring_full(ring)) {
		size_t new_size = (ring->size << 1) | 1;
		TYPE *new_buf = (TYPE *)malloc(new_size * sizeof(TYPE));
		if (!new_buf)
			return true;
		_ax_ring_resize(ring, new_buf, new_size);
	}
	ring->front = ax_cyclic_dec(ring->front, ring->size);
	ring->buf[ring->front] = *value;
	return false;
}

static ax_fail ax_ring_push_back(struct ax_ring_st *ring, TYPE *value)
{
	if (_ax_ring_full(ring)) {
		size_t new_size = (ring->size << 1) | 1;
		TYPE *new_buf = (TYPE *)malloc(new_size * sizeof(TYPE));
		if (!new_buf)
			return true;
		_ax_ring_resize(ring, new_buf, new_size);
	}
	ring->buf[ring->rear] = *value;
	ring->rear = ax_cyclic_inc(ring->rear, ring->size);
	return false;
}

static void ax_ring_pop_front(struct ax_ring_st *ring)
{
	ax_assert(!_ax_ring_empty(ring), "empty");
	ring->front = ax_cyclic_inc(ring->front, ring->size);
}

static void ax_ring_pop_back(struct ax_ring_st *ring)
{
	ax_assert(!_ax_ring_empty(ring), "empty");
	ring->rear = ax_cyclic_dec(ring->rear, ring->size);
}

inline static size_t ax_ring_size(const struct ax_ring_st *ring)
{
	return (ring->rear - ring->front + ring->size) % ring->size;
}

static TYPE *ax_ring_front(const struct ax_ring_st *ring)
{
	ax_assert(!_ax_ring_empty(ring), "empty");
	return ring->buf + ring->front;
}

static TYPE *ax_ring_back(const struct ax_ring_st *ring)
{
	ax_assert(!_ax_ring_empty(ring), "empty");
	return ring->buf + ax_cyclic_dec(ring->rear, ring->size);
}

static TYPE *ax_ring_at(const struct ax_ring_st *ring, size_t index)
{
	ax_assert(index < ax_ring_size(ring), "subscript out of range");
	return ring->buf + (ring->front + index) % ring->size;
}

inline static size_t ax_ring_offset(const struct ax_ring_st *ring, size_t index)
{
	ax_assert(index < ax_ring_size(ring), "subscript out of range");
	return (ring->front + index) % ring->size;
}

inline static size_t ax_ring_step_offset(const struct ax_ring_st *ring, size_t offset, size_t step, bool positive)
{
	return positive
		? (offset + step) % ring->size
		: (offset + ring->size - step) % ring->size;
}

static TYPE *ax_ring_at_offset(const struct ax_ring_st *ring, size_t offset)
{
	ax_assert(ax_ring_offset_is_valid(ring, offset), "offset is invalid");
	return ring->buf + offset;
}

static size_t ax_ring_offset_to_index(const struct ax_ring_st *ring, size_t offset)
{
	ax_assert(ax_ring_offset_is_valid(ring, offset), "offset is invalid");
	return offset >= ring->front
		? offset - ring->front
		: offset + ring->size - ring->front;
}

static void ax_ring_clear(struct ax_ring_st *ring)
{
	free(ring->buf);
	ring->buf = NULL;
	ring->size = 1;
	ring->front = ring->rear = 0;
}

inline static void ax_ring_init(struct ax_ring_st *ring)
{
	{
		ax_unused(ax_ring_free);
		ax_unused(ax_ring_size);
		ax_unused(ax_ring_push_back);
		ax_unused(ax_ring_push_front);
		ax_unused(ax_ring_pop_back);
		ax_unused(ax_ring_pop_front);
		ax_unused(ax_ring_front);
		ax_unused(ax_ring_back);
		ax_unused(ax_ring_at);
		ax_unused(ax_ring_clear);
		ax_unused(ax_ring_offset_is_valid);
	}
	ring->buf = NULL;
	ring->size = 1;
	ring->front = ring->rear = 0;
}

#undef AX_RING
#undef ax_ring_st
#undef ax_ring_init
#undef ax_ring_free
#undef ax_ring_size
#undef ax_ring_push_back
#undef ax_ring_push_front
#undef ax_ring_pop_back
#undef ax_ring_pop_front
#undef ax_ring_front
#undef ax_ring_back
#undef ax_ring_at
#undef ax_ring_clear
#undef ax_ring_offset_is_valid
#undef ax_ring_offset
#undef ax_ring_at_offset
#undef ax_ring_at_offset
#undef ax_ring_offset_to_index
#undef _ax_ring_full
#undef _ax_ring_empty
#undef _ax_ring_resize
#undef TYPE
#undef NAME

