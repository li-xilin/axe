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

#include <ax/deq.h>
#include <ax/def.h>
#include <ax/any.h>
#include <ax/iter.h>
#include <ax/debug.h>
#include <ax/trait.h>
#include <ax/class.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include "ax/mem.h"
#include "check.h"

#define NAME
#define TYPE ax_byte *
#include "ring.h"

//#define BLOCK_SIZE 0x400
#define BLOCK_SIZE 8

#undef free

AX_CLASS_STRUCT_ENTRY(deq)
	struct ring_st map;
	uint16_t front, rear;
AX_END;

struct position
{
	size_t midx;
	size_t boff;
};

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap);
static ax_fail seq_pop(ax_seq *seq);
static ax_fail seq_pushf(ax_seq *seq, const void *val, va_list *ap);
static ax_fail seq_popf(ax_seq *seq);
static ax_fail seq_trunc(ax_seq *seq, size_t size);
static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap);
static ax_iter seq_at(const ax_seq *seq, size_t midx);
static void   *seq_last(const ax_seq *seq);
static void   *seq_first(const ax_seq *seq);


static size_t  box_size(const ax_box *box);
static size_t  box_maxsize(const ax_box *box);
static ax_iter box_begin(ax_box *box);
static ax_iter box_end(ax_box *box);
static ax_iter box_rbegin(ax_box *box);
static ax_iter box_rend(ax_box *box);
static void    box_clear(ax_box *box);

static ax_any *any_copy(const ax_any *any);

static void    one_free(ax_one *one);

static void    citer_prev(ax_citer *it);
static void    citer_next(ax_citer *it);
static bool    citer_less(const ax_citer *it1, const ax_citer *it2);
static long    citer_dist(const ax_citer *it1, const ax_citer *it2);

static bool    rciter_less(const ax_citer *it1, const ax_citer *it2);
static long    rciter_dist(const ax_citer *it1, const ax_citer *it2);

static void   *citer_get(const ax_citer *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);
static void    iter_erase(ax_iter *it);

inline static bool have_previous_pos(ax_deq *deq, size_t midx, size_t boff)
{
	return (boff != 0 || boff != 0);
}

inline static bool have_follow_pos(ax_deq *deq, size_t midx, size_t boff)
{
	return (midx != ring_size(&deq->map) - 1 || boff != BLOCK_SIZE - 1);
}

inline static ax_byte *get_value(const ax_deq *deq, size_t midx, size_t boff)
{
	return *ring_at(&deq->map, midx) + boff * ax_cr(deq, deq).box->env.elem_tr->size;
}

inline static void move_position(const ax_deq *deq, size_t *midx, size_t *boff, int step)
{
	size_t amount = *midx * BLOCK_SIZE + *boff + step;
	*midx = amount / BLOCK_SIZE;
	*boff = amount % BLOCK_SIZE;
}

inline static bool prev_position(const ax_deq *deq, size_t *midx, size_t *boff)
{
	if (*boff == 0) {
		(*midx)--;
		*boff = BLOCK_SIZE - 1;
		return true;
	}
	(*boff)--;
	return false;
}

inline static bool next_position(const ax_deq *deq, size_t *midx, size_t *boff)
{
	if (*boff == BLOCK_SIZE - 1) {
		(*midx)++;
		*boff = 0;
		return true;
	}
	(*boff)++;
	return false;
}

static void citer_move(ax_citer *it, long s)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);
	const ax_deq *self = it->owner;
	const ax_byte *block = *ring_at_offset(&self->map, it->extra);
	ax_byte *current = it->point;

	size_t midx = ring_offset_to_index(&self->map, it->extra);
	size_t boff = (current - block) / it->etr->size;
	
	move_position(self, &midx, &boff, it->tr->norm ? s : - s);

	it->extra = ring_offset(&self->map, midx);
	it->point = get_value(self, midx, boff);
}

static void citer_prev(ax_citer *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);
	citer_move(it, -1);
}

static void citer_next(ax_citer *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);
	citer_move(it, 1);
}

static void *citer_get(const ax_citer *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);
	return it->point;
}

static bool citer_less(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	if (it1->extra != it2->extra)
		return it1->extra < it2->extra;
	return it1->point < it2->point;
}

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	
	return 0;
}

static bool rciter_less(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	if (it1->extra != it2->extra)
		return it1->extra > it2->extra;
	return it1->point > it2->point;
}

static long rciter_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	return 0;
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);
	return ax_trait_copy_or_init(it->etr, it->point, val, ap);
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

}

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ax_deq_r self = { .one = one };
	for (size_t i = 0; i < ring_size(&self.deq->map); i++)
		free(ring_at(&self.deq->map, i));
	ring_free(&self.deq->map);
	free(one);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_seq_cr self = { .any = any };
	return ax_seq_dump(self.seq);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	return NULL;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_cr self = { .box = box };
	return ring_size(&self.deq->map) * BLOCK_SIZE
		- (self.deq->front + 1)
		- (BLOCK_SIZE - self.deq->rear);
}

static size_t box_maxsize(const ax_box *box)
{
	return SIZE_MAX;
}


static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = { .box = box };
	size_t midx = 0, boff = self.deq->front;
	next_position(self.deq, &midx, &boff);
	return (ax_iter) {
		.owner = box,
		.point = get_value(self.deq, midx, boff),
		.extra = ring_offset(&self.deq->map, midx),
		.tr = &ax_deq_tr.box.iter,
		.etr = box->env.elem_tr,
	};
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = { .box = box };
	size_t midx = ring_size(&self.deq->map) - 1;
	size_t boff = self.deq->rear;
	return (ax_iter) {
		.owner = box,
		.point = get_value(self.deq, midx, boff),
		.extra = ring_offset(&self.deq->map, midx),
		.tr = &ax_deq_tr.box.iter,
		.etr = box->env.elem_tr,
	};
	
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = { .box = box };
	size_t midx = ring_size(&self.deq->map) - 1;
	size_t boff = self.deq->rear;
	prev_position(self.deq, &midx, &boff);
	return (ax_iter) {
		.owner = box,
		.point = get_value(self.deq, midx, boff),
		.extra = ring_offset(&self.deq->map, midx),
		.tr = &ax_deq_tr.box.riter,
		.etr = box->env.elem_tr,
	};
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = { .box = box };
	size_t midx = 0, boff = self.deq->front;
	return (ax_iter) {
		.owner = box,
		.point = get_value(self.deq, midx, boff),
		.extra = ring_offset(&self.deq->map, midx),
		.tr = &ax_deq_tr.box.riter,
		.etr = box->env.elem_tr,
	};
}

#define RESET_FRONT_AND_REAR(deq) \
	do { \
		size_t middle = BLOCK_SIZE / 2; \
		(deq)->front = middle; \
		(deq)->rear = middle + 1; \
	} while (0)

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = { .box = box };
	size_t map_size = ring_size(&self.deq->map);

	size_t midx = 0, boff = self.deq->front;
	next_position(self.deq, &midx, &boff);
	while (midx != map_size - 1 && boff != self.deq->rear) {
		ax_trait_free(box->env.elem_tr, get_value(self.deq, midx, boff));
		next_position(self.deq, &midx, &boff);
	}

	ax_byte *block = *ring_at(&self.deq->map, 0);
	for (size_t i = 1; i < map_size - 1; i++)
		free(*ring_at(&self.deq->map, i));
	ring_clear(&self.deq->map);

	ring_push_back(&self.deq->map, &block);

	RESET_FRONT_AND_REAR(self.deq);
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_deq_r self = { .seq = seq };
	size_t midx = ring_size(&self.deq->map) - 1, boff = self.deq->rear;

	size_t new_rear_midx = midx, new_rear_boff = boff;
	move_position(self.deq, &new_rear_midx, &new_rear_boff, 1);

	bool block_added = false;
	if (!have_follow_pos(self.deq, midx, boff)) {
		ax_byte *block = malloc(BLOCK_SIZE * self.box->env.elem_tr->size);
		if (!block)
			return true;
		if (ring_push_back(&self.deq->map, &block)) {
			free(block);
			return true;
		}
		block_added = true;
	}

	const ax_trait *etr = self.box->env.elem_tr;
	ax_byte tmp[etr->size];
	if (ax_trait_copy_or_init(etr, tmp, val, ap)) {
		if (block_added) {
			free(*ring_back(&self.deq->map));
			ring_pop_back(&self.deq->map);
		}
		return true;
	}

	const ax_byte *block = *ring_at_offset(&self.deq->map, it->extra);
	ax_byte *current = it->point;
	size_t target_midx = ring_offset_to_index(&self.deq->map, it->extra),
		   target_boff = (current - block) / it->etr->size;

	while (midx != target_midx || boff != target_boff) {
		size_t midx1 = midx, boff1 = boff;
		move_position(self.deq, &midx, &boff, -1);
		void *src = get_value(self.deq, midx, boff),
			 *dst = get_value(self.deq, midx1, boff1);
		memcpy(dst, src, it->etr->size);
	}
	memcpy(get_value(self.deq, target_midx, target_boff), tmp, etr->size);
	self.deq->rear = new_rear_boff;

	move_position(self.deq, &target_midx, &target_boff, 1);
	it->extra = ring_offset(&self.deq->map, target_midx);
	it->point = get_value(self.deq, target_midx, target_boff);
	return false;
}

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = { .seq = seq };
	size_t midx = ring_size(&self.deq->map) - 1, boff = self.deq->rear;
	bool block_added = false;
	if (!have_follow_pos(self.deq, midx, boff)) {
		ax_byte *block = malloc(BLOCK_SIZE * self.box->env.elem_tr->size);
		if (!block)
			return true;
		if (ring_push_back(&self.deq->map, &block)) {
			free(block);
			return true;
		}
		block_added = true;
	}
	if (ax_trait_copy_or_init(self.box->env.elem_tr,
				get_value(self.deq, midx, boff), val, ap)) {
		if (block_added) {
			free(*ring_back(&self.deq->map));
			ring_pop_back(&self.deq->map);
		}
		return true;
	}
	move_position(self.deq, &midx, &boff, 1);
	self.deq->rear = boff;
	return false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = { .seq = seq };
	size_t midx = ring_size(&self.deq->map) - 1, boff = self.deq->rear;
	if (prev_position(self.deq, &midx, &boff)) 
		ring_pop_back(&self.deq->map);
	ax_trait_free(self.box->env.elem_tr, get_value(self.deq, midx, boff));
	self.deq->rear = boff;
	return false;
}

static ax_fail seq_pushf(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = { .seq = seq };
	size_t midx = 0, boff = self.deq->front;
	bool block_added = false;
	if (!have_previous_pos(self.deq, midx, boff)) {
		ax_byte *block = malloc(BLOCK_SIZE * self.box->env.elem_tr->size);
		if (!block)
			return true;
		if (ring_push_front(&self.deq->map, &block)) {
			free(block);
			return true;
		}
		midx++;
		block_added = true;
	}
	if (ax_trait_copy_or_init(self.box->env.elem_tr,
				get_value(self.deq, midx, boff), val, ap)) {
		if (block_added)
			ring_pop_front(&self.deq->map);
		return true;
	}
	move_position(self.deq, &midx, &boff, -1);
	self.deq->front = boff;
	return false;
}

static ax_fail seq_popf(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = { .seq = seq };
	size_t midx = 0, boff = self.deq->front;
	if (next_position(self.deq, &midx, &boff)) {
		ring_pop_front(&self.deq->map);
		midx = 0;
	}
	ax_trait_free(self.box->env.elem_tr, get_value(self.deq, midx, boff));
	self.deq->front = boff;
	return false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = { .seq = seq };

	size_t map_size = ring_size(&self.deq->map);

	size_t index1 = 0, offset1 = self.deq->front,
		   index2 = map_size - 1, offset2 = self.deq->rear;

	next_position(self.deq, &index1, &offset1);
	prev_position(self.deq, &index2, &offset2);

	size_t size = box_size(self.box);

	for (size_t i = 0; i < size / 2; i++) {
		ax_memswp(get_value(self.deq, index1, offset1), get_value(self.deq, index2, offset2),
				self.box->env.elem_tr->size);
		next_position(self.deq, &index1, &offset1);
		prev_position(self.deq, &index2, &offset2);
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= box_maxsize(ax_r(seq, seq).box));
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t idx)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_cr self = { .seq = seq };
	size_t amount = self.deq->front + idx + 1;
	size_t midx = amount / BLOCK_SIZE, boff = amount % BLOCK_SIZE;
	return (ax_iter) {
		.owner = (void *)self.deq,
		.point = get_value(self.deq, midx, boff),
		.extra = ring_offset(&self.deq->map, midx),
		.tr = &ax_deq_tr.box.iter,
		.etr = self.box->env.elem_tr,
	};
}

static void *seq_last(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_cr self = { .seq = seq };
	size_t midx = ring_size(&self.deq->map) - 1, boff = self.deq->rear;
	move_position(self.deq, &midx, &boff, -1);
	return get_value(self.deq, midx, boff);
}

static void *seq_first(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_cr self = { .seq = seq };
	size_t midx = 0, boff = self.deq->front;
	move_position(self.deq, &midx, &boff, 1);
	return get_value(self.deq, midx, boff);
}

const ax_seq_trait ax_deq_tr =
{
	.box = {
		.any = {
			.one = {
				.name = AX_DEQ_NAME,
				.free = one_free,
			},
			.dump = any_dump,
			.copy = any_copy,
		},
		.iter = {
			.norm = true,
			.type = AX_IT_BID,
			.move = NULL,
			.next = citer_next,
			.prev = citer_prev,
			.less = citer_less,
			.dist = citer_dist,
			.get = citer_get,
			.set = iter_set,
			.erase = iter_erase,
		},
		.riter = {
			.norm = false,
			.type = AX_IT_BID,
			.move = NULL,
			.prev = citer_prev,
			.next = citer_next,
			.less = rciter_less,
			.dist = rciter_dist,
			.get = citer_get,
			.set = iter_set,
			.erase = iter_erase,
		},

		.size = box_size,
		.maxsize = box_maxsize,

		.begin = box_begin,
		.end = box_end,
		.rbegin = box_rbegin,
		.rend = box_rend,

		.clear = box_clear,

	},
	.push = seq_push,
	.pop = seq_pop,
	.pushf = seq_pushf,
	.popf = seq_popf,
	.invert = seq_invert,
	.trunc = seq_trunc,
	.insert = seq_insert,
	.at = seq_at,
	.first = seq_first,
	.last = seq_last,
};


ax_seq* __ax_deq_construct(const ax_trait *elem_tr)
{
	CHECK_PARAM_NULL(elem_tr);

	CHECK_PARAM_NULL(elem_tr->free);
	CHECK_PARAM_NULL(elem_tr->copy);

	
	ax_deq deq = {
		.seq = {
			.tr = &ax_deq_tr,
			.env.box.elem_tr = elem_tr,
		},
	};
	RESET_FRONT_AND_REAR(&deq);

	ring_init(&deq.map);


	ax_byte *block = NULL;
	ax_deq_r self = ax_null;

	self.deq = malloc(sizeof(ax_deq));
	if (!self.deq)
		goto fail;

	block = malloc(BLOCK_SIZE * elem_tr->size);
	if (!block)
		goto fail;

	if (ring_push_back(&deq.map, &block))
		goto fail;

	memcpy(self.deq, &deq, sizeof deq);
	return self.seq;
fail:
	free(self.deq);
	free(block);
	ring_free(&deq.map);
	return NULL;
}

