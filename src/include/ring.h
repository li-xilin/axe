#include "ax/def.h"
#include "ax/trick.h"
#include <stdlib.h>
#include <string.h>

#ifndef TYPE
#error "TYPE is not defined"
#define TYPE int
#endif

#ifndef NAME
#error "NAME is not defined"
#define NAME
#endif

#define AX_RING(tail) AX_CATENATE(NAME, ring_, tail)
#define _AX_RING(tail) AX_CATENATE(_, NAME, ring_, tail)
#define ring_st AX_RING(st)
#define ring_init AX_RING(init)
#define ring_free AX_RING(free)
#define ring_size AX_RING(size)
#define ring_push_back AX_RING(push_back)
#define ring_push_front AX_RING(push_front)
#define ring_pop_back AX_RING(pop_back)
#define ring_pop_front AX_RING(pop_front)
#define ring_front AX_RING(front)
#define ring_back AX_RING(back)
#define ring_at AX_RING(at)
#define ring_clear AX_RING(clear)
#define ring_offset_is_valid AX_RING(offset_is_valid)

#define _ring_full(ring) ((ring->rear + 1) % ring->size == ring->front)
#define _ring_empty(ring) (ring->front == ring->rear)
#define _ring_resize _AX_RING(resize)

struct ring_st
{
	TYPE *buf;
	size_t size;
	size_t front, rear;
};

inline static size_t ring_size(const struct ring_st *ring);

static bool ring_offset_is_valid(const struct ring_st *ring, size_t offset)
{
	//return (ring->rear >= ring->front)
	//	? offset >= ring->front && offset < ring->rear
	//	: offset >= ring->front || offset < ring->rear;
	return offset >= ring->front
		? ring->rear > offset || ring->rear < ring->front
		: ring->rear > offset;
}

static void _ring_resize(struct ring_st *ring, TYPE *new_buf, size_t new_size)
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

static void ring_free(struct ring_st *ring)
{
	if (!ring)
		return;
	free(ring->buf);
}

static ax_fail ring_push_front(struct ring_st *ring, TYPE *value)
{
	if (_ring_full(ring)) {
		size_t new_size = (ring->size << 1) | 1;
		TYPE *new_buf = (TYPE *)malloc(new_size * sizeof(TYPE));
		if (!new_buf)
			return true;
		_ring_resize(ring, new_buf, new_size);
	}
	ring->front = ax_cyclic_dec(ring->front, ring->size);
	ring->buf[ring->front] = *value;
	return false;
}

static ax_fail ring_push_back(struct ring_st *ring, TYPE *value)
{
	if (_ring_full(ring)) {
		size_t new_size = (ring->size << 1) | 1;
		TYPE *new_buf = (TYPE *)malloc(new_size * sizeof(TYPE));
		if (!new_buf)
			return true;
		_ring_resize(ring, new_buf, new_size);
	}
	ring->buf[ring->rear] = *value;
	ring->rear = ax_cyclic_inc(ring->rear, ring->size);
	return false;
}

static void ring_pop_front(struct ring_st *ring)
{
	ax_assert(!_ring_empty(ring), "empty");
	ring->front = ax_cyclic_inc(ring->front, ring->size);
}

static void ring_pop_back(struct ring_st *ring)
{
	ax_assert(!_ring_empty(ring), "empty");
	ring->rear = ax_cyclic_dec(ring->rear, ring->size);
}

inline static size_t ring_size(const struct ring_st *ring)
{
	return (ring->rear - ring->front + ring->size) % ring->size;
}

static TYPE *ring_front(const struct ring_st *ring)
{
	ax_assert(!_ring_empty(ring), "empty");
	return ring->buf + ring->front;
}

static TYPE *ring_back(const struct ring_st *ring)
{
	ax_assert(!_ring_empty(ring), "empty");
	return ring->buf + ax_cyclic_dec(ring->rear, ring->size);
}

static TYPE *ring_at(const struct ring_st *ring, size_t index)
{
	ax_assert(index < ring_size(ring), "subscript out of range");
	return ring->buf + (ring->front + index) % ring->size;
}

inline static size_t ring_offset(const struct ring_st *ring, size_t index)
{
	ax_assert(index < ring_size(ring), "subscript out of range");
	return (ring->front + index) % ring->size;
}

/*
inline static size_t ring_prev_offset(const struct ring_st *ring, size_t offset)
{
	return (offset + ring->size - 1) % ring->size;
}

inline static size_t ring_next_offset(const struct ring_st *ring, size_t offset)
{
	return (offset + 1) % ring->size;
}
*/

inline static size_t ring_step_offset(const struct ring_st *ring, size_t offset, size_t step, bool positive)
{
	return positive
		? (offset + step) % ring->size
		: (offset + ring->size - step) % ring->size;

}

static TYPE *ring_at_offset(const struct ring_st *ring, size_t offset)
{
	ax_assert(ring_offset_is_valid(ring, offset), "offset is invalid");
	return ring->buf + offset;
}

static size_t ring_offset_to_index(const struct ring_st *ring, size_t offset)
{
	ax_assert(ring_offset_is_valid(ring, offset), "offset is invalid");
	return offset >= ring->front
		? offset - ring->front
		: offset + ring->size - ring->front;
}

static void ring_clear(struct ring_st *ring)
{
	free(ring->buf);
	ring->buf = NULL;
	ring->size = 1;
	ring->front = ring->rear = 0;
}

void ring_init(struct ring_st *ring)
{
	{
		ax_unused(ring_free);
		ax_unused(ring_size);
		ax_unused(ring_push_back);
		ax_unused(ring_push_front);
		ax_unused(ring_pop_back);
		ax_unused(ring_pop_front);
		ax_unused(ring_front);
		ax_unused(ring_back);
		ax_unused(ring_at);
		ax_unused(ring_clear);
		ax_unused(ring_offset_is_valid);
	}
	ring->buf = NULL;
	ring->size = 1;
	ring->front = ring->rear = 0;
}

#undef AX_RING
#undef ring_st
#undef ring_init
#undef ring_free
#undef ring_size
#undef ring_push_back
#undef ring_push_front
#undef ring_pop_back
#undef ring_pop_front
#undef ring_front
#undef ring_back
#undef ring_at
#undef ring_clear
#undef ring_offset_is_valid
#undef _ring_full
#undef _ring_empty
#undef _ring_resize
#undef TYPE
#undef NAME

