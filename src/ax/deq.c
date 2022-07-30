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

#define NAME
#define TYPE ax_byte *
#include "ring.h"

#include "ax/deq.h"
#include "ax/mem.h"
#include "ax/def.h"
#include "ax/iter.h"
#include "ax/trait.h"
#include "check.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <math.h>

#define BLOCK_SIZE 0x400

#define RESET_FRONT_AND_REAR(deq) \
	do { \
		size_t middle = BLOCK_SIZE / 2; \
		(deq)->front = middle; \
		(deq)->rear = middle + 1; \
	} while (0)

#define INDEX_BY_POS(deq, pos) \
	((((pos.midx) + 1) * BLOCK_SIZE) - ((deq)->front + 1) - (BLOCK_SIZE - (pos.boff)))

#define POS_EQUAL(pos1, pos2) \
	(pos1.midx == pos2.midx && pos1.boff == pos2.boff)

#undef free

ax_concrete_begin(ax_deq)
	struct ring_st map;
	uint16_t front, rear;
ax_end;

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
static const char *one_name(const ax_one *one);

static void    citer_prev(ax_citer *it);
static void    citer_next(ax_citer *it);
static bool    citer_less(const ax_citer *it1, const ax_citer *it2);
static long    citer_dist(const ax_citer *it1, const ax_citer *it2);

static bool    rciter_less(const ax_citer *it1, const ax_citer *it2);
static long    rciter_dist(const ax_citer *it1, const ax_citer *it2);

static void   *citer_get(const ax_citer *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);
static void    iter_erase(ax_iter *it);

inline static bool have_previous_pos(ax_deq *deq, const struct position *pos)
{
	return (pos->boff != 0 || pos->boff != 0);
}

inline static bool have_follow_pos(ax_deq *deq, const struct position *pos)
{
	return (pos->midx != ring_size(&deq->map) - 1 || pos->boff != BLOCK_SIZE - 1);
}

inline static struct position front_end_position(ax_deq *deq)
{
	return (struct position) { 0, deq->front, };
}

inline static struct position back_end_position(ax_deq *deq)
{
	return (struct position) { ring_size(&deq->map) - 1, deq->rear, };
}

inline static ax_byte *position_ptr(const ax_deq *deq, const struct position *pos)
{
	return *ring_at(&deq->map, pos->midx) + pos->boff * ax_class_env(ax_cr(ax_deq, deq).ax_box).elem_tr->size;
}

inline static ptrdiff_t move_position(const ax_deq *deq, struct position *pos, ptrdiff_t step)
{
	size_t size = box_size(ax_cr(ax_deq, deq).ax_box);
	ptrdiff_t amount = pos->midx * BLOCK_SIZE + pos->boff + step;
	ptrdiff_t extend = 0;

	if (amount < 0) {
		extend = amount / BLOCK_SIZE - 1;
		amount = size - - amount % size;
	} else if (amount >= size) {
		extend = amount / BLOCK_SIZE + 1 - ring_size(&deq->map);
	}
	pos->midx = amount / BLOCK_SIZE;
	pos->boff = amount % BLOCK_SIZE;
	return extend;
}

inline static bool prev_position(const ax_deq *deq, struct position *pos)
{
	if (pos->boff == 0) {
		(pos->midx)--;
		pos->boff = BLOCK_SIZE - 1;
		return true;
	}
	(pos->boff)--;
	return false;
}

inline static bool next_position(const ax_deq *deq, struct position *pos)
{
	if (pos->boff == BLOCK_SIZE - 1) {
		(pos->midx)++;
		pos->boff = 0;
		return true;
	}
	(pos->boff)++;
	return false;
}

inline static void iter_set_pos(ax_citer *it, const struct position *pos)
{
	const ax_deq *deq = it->owner;
	it->extra = ring_offset(&deq->map, pos->midx);
	it->point = position_ptr(deq, pos);
}

inline static struct position get_position_by_iter(const ax_citer *it)
{
	const ax_deq *self = it->owner;
	return (struct position) {
		ring_offset_to_index(&self->map, it->extra),
		((ax_byte *)it->point - *ring_at_offset(&self->map, it->extra)) / it->etr->size,
	};
}

static void citer_move(ax_citer *it, long s)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);
	const ax_deq *self = it->owner;
	
	struct position pos = get_position_by_iter(it);
	
	size_t miss = move_position(self, &pos, it->tr->norm ? s : - s);
	ax_unused(miss);
	ax_assert(!miss, "the position where iterator moved to is not existed");

	it->extra = ring_offset(&self->map, pos.midx);
	it->point = position_ptr(self, &pos);
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

	const ax_deq *self = it1->owner;

	struct position
		pos1 = get_position_by_iter(it1),
		pos2 = get_position_by_iter(it2);

	size_t
		index1 = INDEX_BY_POS(self, pos1),
		index2 = INDEX_BY_POS(self, pos2);

	return index2 - index1;
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

	return - citer_dist(it1, it2);
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	ax_seq_cr self = AX_R_INIT(ax_one, it->owner);
	const ax_trait *etr = ax_class_env(self.ax_box).elem_tr;
	ax_byte tmp[etr->size];
	if (ax_trait_copy_or_init(it->etr, tmp, val, ap))
		return true;
	ax_trait_free(etr, it->point);
	ax_memswp(tmp, it->point, etr->size);
	return false;
}


static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_deq_r self = AX_R_INIT(ax_one, it->owner);
	struct position pos = get_position_by_iter(ax_iter_c(it));

	struct position next_pos = pos;
	move_position(self.ax_deq, &next_pos, 1);

	struct position ret_pos = pos;

	struct position end = back_end_position(self.ax_deq);
	void *val = position_ptr(self.ax_deq, &pos);
	const ax_trait *etr = ax_class_env(self.ax_box).elem_tr;
	ax_trait_free(etr, val);
	while (!POS_EQUAL(next_pos, end)) {
		void *next_val = position_ptr(self.ax_deq, &next_pos);
		memcpy(val, next_val, etr->size);
		val = next_val;
		pos = next_pos;
		move_position(self.ax_deq, &next_pos, 1);
	}
	if (next_pos.midx != pos.midx)
		ring_pop_back(&self.ax_deq->map);

	struct position new_rear = end;
	move_position(self.ax_deq, &new_rear, -1);
	if (new_rear.midx != pos.midx)
		ring_pop_back(&self.ax_deq->map);
	self.ax_deq->rear = new_rear.boff;

	iter_set_pos(ax_iter_c(it), &ret_pos);
}

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ax_deq_r self = AX_R_INIT(ax_one, one);
	for (size_t i = 0; i < ring_size(&self.ax_deq->map); i++)
		free(*ring_at(&self.ax_deq->map, i));
	ring_free(&self.ax_deq->map);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, ax_deq);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_seq_cr self = AX_R_INIT(ax_any, any);
	return ax_seq_dump(self.ax_seq);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);


	return NULL;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_cr self = AX_R_INIT(ax_box, box);
	return ring_size(&self.ax_deq->map) * BLOCK_SIZE
		- (self.ax_deq->front + 1)
		- (BLOCK_SIZE - self.ax_deq->rear);
}

static size_t box_maxsize(const ax_box *box)
{
	return PTRDIFF_MAX;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = AX_R_INIT(ax_box, box);
	struct position pos = { 0, self.ax_deq->front, };
	next_position(self.ax_deq, &pos);
	return (ax_iter) {
		.owner = box,
		.point = position_ptr(self.ax_deq, &pos),
		.extra = ring_offset(&self.ax_deq->map, pos.midx),
		.tr = &ax_deq_tr.ax_box.iter,
		.etr = box->env.elem_tr,
	};
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = AX_R_INIT(ax_box, box);
	struct position pos = { ring_size(&self.ax_deq->map) - 1, self.ax_deq->rear, };
	return (ax_iter) {
		.owner = box,
		.point = position_ptr(self.ax_deq, &pos),
		.extra = ring_offset(&self.ax_deq->map, pos.midx),
		.tr = &ax_deq_tr.ax_box.iter,
		.etr = box->env.elem_tr,
	};
	
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = AX_R_INIT(ax_box, box);
	struct position pos = back_end_position(self.ax_deq);
	move_position(self.ax_deq, &pos, -1);
	return (ax_iter) {
		.owner = box,
		.point = position_ptr(self.ax_deq, &pos),
		.extra = ring_offset(&self.ax_deq->map, pos.midx),
		.tr = &ax_deq_tr.ax_box.riter,
		.etr = box->env.elem_tr,
	};
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = AX_R_INIT(ax_box, box);
	struct position pos = front_end_position(self.ax_deq);
	return (ax_iter) {
		.owner = box,
		.point = position_ptr(self.ax_deq, &pos),
		.extra = ring_offset(&self.ax_deq->map, pos.midx),
		.tr = &ax_deq_tr.ax_box.riter,
		.etr = box->env.elem_tr,
	};
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_deq_r self = AX_R_INIT(ax_box, box);
	size_t map_size = ring_size(&self.ax_deq->map);

	struct position pos = { 0, self.ax_deq->front, };
	next_position(self.ax_deq, &pos);
	while (pos.midx != map_size - 1 && pos.boff != self.ax_deq->rear) {
		ax_trait_free(box->env.elem_tr, position_ptr(self.ax_deq, &pos));
		next_position(self.ax_deq, &pos);
	}

	ax_byte *block = *ring_at(&self.ax_deq->map, 0);
	for (size_t i = 1; i < map_size - 1; i++)
		free(*ring_at(&self.ax_deq->map, i));
	ring_clear(&self.ax_deq->map);

	ring_push_back(&self.ax_deq->map, &block);

	RESET_FRONT_AND_REAR(self.ax_deq);
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_deq_r self = AX_R_INIT(ax_seq, seq);
	struct position pos = { ring_size(&self.ax_deq->map) - 1, self.ax_deq->rear, };

	struct position new_rear_pos = pos;
	move_position(self.ax_deq, &new_rear_pos, 1);

	bool block_added = false;
	if (!have_follow_pos(self.ax_deq, &pos)) {
		ax_byte *block = malloc(BLOCK_SIZE * self.ax_box->env.elem_tr->size);
		if (!block)
			return true;
		if (ring_push_back(&self.ax_deq->map, &block)) {
			free(block);
			return true;
		}
		block_added = true;
	}

	const ax_trait *etr = self.ax_box->env.elem_tr;
	ax_byte tmp[etr->size];
	if (ax_trait_copy_or_init(etr, tmp, val, ap)) {
		if (block_added) {
			free(*ring_back(&self.ax_deq->map));
			ring_pop_back(&self.ax_deq->map);
		}
		return true;
	}

	const ax_byte *block = *ring_at_offset(&self.ax_deq->map, it->extra);
	ax_byte *current = it->point;
	struct position target_pos = {
		ring_offset_to_index(&self.ax_deq->map, it->extra),
		(current - block) / it->etr->size,
	};

	while (!POS_EQUAL(pos, target_pos)) {
		struct position pos1 = pos;
		move_position(self.ax_deq, &pos, -1);
		void *src = position_ptr(self.ax_deq, &pos),
			 *dst = position_ptr(self.ax_deq, &pos1);
		memcpy(dst, src, it->etr->size);
	}
	memcpy(position_ptr(self.ax_deq, &target_pos), tmp, etr->size);
	self.ax_deq->rear = new_rear_pos.boff;

	move_position(self.ax_deq, &target_pos, 1);
	iter_set_pos(ax_iter_c(it), &target_pos);
	return false;
}

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = AX_R_INIT(ax_seq, seq);
	struct position pos = { ring_size(&self.ax_deq->map) - 1, self.ax_deq->rear };
	bool block_added = false;
	if (!have_follow_pos(self.ax_deq, &pos)) {
		ax_byte *block = malloc(BLOCK_SIZE * self.ax_box->env.elem_tr->size);
		if (!block)
			return true;
		if (ring_push_back(&self.ax_deq->map, &block)) {
			free(block);
			return true;
		}
		block_added = true;
	}
	if (ax_trait_copy_or_init(self.ax_box->env.elem_tr,
				position_ptr(self.ax_deq, &pos), val, ap)) {
		if (block_added) {
			free(*ring_back(&self.ax_deq->map));
			ring_pop_back(&self.ax_deq->map);
		}
		return true;
	}
	move_position(self.ax_deq, &pos, 1);
	self.ax_deq->rear = pos.boff;
	return false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = AX_R_INIT(ax_seq, seq);
	struct position pos = { ring_size(&self.ax_deq->map) - 1, self.ax_deq->rear, };
	if (prev_position(self.ax_deq, &pos)) 
		ring_pop_back(&self.ax_deq->map);
	ax_trait_free(self.ax_box->env.elem_tr, position_ptr(self.ax_deq, &pos));
	self.ax_deq->rear = pos.boff;
	return false;
}

static ax_fail seq_pushf(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = AX_R_INIT(ax_seq, seq);
	struct position pos = { 0,  self.ax_deq->front, };
	bool block_added = false;
	if (!have_previous_pos(self.ax_deq, &pos)) {
		ax_byte *block = malloc(BLOCK_SIZE * self.ax_box->env.elem_tr->size);
		if (!block)
			return true;
		if (ring_push_front(&self.ax_deq->map, &block)) {
			free(block);
			return true;
		}
		pos.midx++;
		block_added = true;
	}
	if (ax_trait_copy_or_init(self.ax_box->env.elem_tr,
				position_ptr(self.ax_deq, &pos), val, ap)) {
		if (block_added)
			ring_pop_front(&self.ax_deq->map);
		return true;
	}
	move_position(self.ax_deq, &pos, -1);
	self.ax_deq->front = pos.boff;
	return false;
}

static ax_fail seq_popf(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = AX_R_INIT(ax_seq, seq);
	struct position pos = { 0, self.ax_deq->front, };
	if (next_position(self.ax_deq, &pos)) {
		ring_pop_front(&self.ax_deq->map);
		pos.midx = 0;
	}
	ax_trait_free(self.ax_box->env.elem_tr, position_ptr(self.ax_deq, &pos));
	self.ax_deq->front = pos.boff;
	return false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_r self = AX_R_INIT(ax_seq, seq);

	size_t map_size = ring_size(&self.ax_deq->map);

	struct position pos1 = { 0, self.ax_deq->front, },
					pos2 = { map_size - 1, self.ax_deq->rear, };

	next_position(self.ax_deq, &pos1);
	prev_position(self.ax_deq, &pos2);

	size_t size = box_size(self.ax_box);

	for (size_t i = 0; i < size / 2; i++) {
		ax_memswp(position_ptr(self.ax_deq, &pos1), position_ptr(self.ax_deq, &pos2),
				self.ax_box->env.elem_tr->size);
		next_position(self.ax_deq, &pos1);
		prev_position(self.ax_deq, &pos2);
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= box_maxsize(ax_r(ax_seq, seq).ax_box));
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t idx)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_cr self = AX_R_INIT(ax_seq, seq);
	size_t amount = self.ax_deq->front + idx + 1;
	struct position pos = { amount / BLOCK_SIZE, amount % BLOCK_SIZE, };
	ax_iter it = {
		.owner = (void *)seq,
		.tr = &ax_deq_tr.ax_box.iter,
		.etr = self.ax_box->env.elem_tr,
	};
	iter_set_pos((ax_citer *)&it, &pos);
	return it;
}

static void *seq_last(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_cr self = AX_R_INIT(ax_seq, seq);
	struct position pos = { ring_size(&self.ax_deq->map) - 1, self.ax_deq->rear, };
	move_position(self.ax_deq, &pos, -1);
	return position_ptr(self.ax_deq, &pos);
}

static void *seq_first(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_deq_cr self = { .ax_seq = seq };
	struct position pos = { 0, self.ax_deq->front, };
	move_position(self.ax_deq, &pos, 1);
	return position_ptr(self.ax_deq, &pos);
}

const ax_seq_trait ax_deq_tr =
{
	.ax_box = {
		.ax_any = {
			.ax_one = {
				.name = one_name,
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
		.ax_seq = {
			.tr = &ax_deq_tr,
			.env.ax_box.elem_tr = elem_tr,
		},
	};
	RESET_FRONT_AND_REAR(&deq);

	ring_init(&deq.map);


	ax_byte *block = NULL;
	ax_deq_r self = AX_R_NULL;
	self.ax_deq = malloc(sizeof(ax_deq));
	if (ax_r_isnull(self))
		goto fail;

	block = malloc(BLOCK_SIZE * elem_tr->size);
	if (!block)
		goto fail;

	if (ring_push_back(&deq.map, &block))
		goto fail;

	memcpy(self.ax_deq, &deq, sizeof deq);
	return self.ax_seq;
fail:
	free(self.ax_deq);
	free(block);
	ring_free(&deq.map);
	return NULL;
}

