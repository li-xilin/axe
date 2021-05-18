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

#include <ax/arr.h>
#include <ax/base.h>
#include <ax/def.h>
#include <ax/iter.h>
#include <ax/debug.h>
#include <ax/vail.h>
#include <ax/stuff.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "check.h"

#undef free

#define MIN_SIZE

static void    seq_invert(ax_seq *seq);
static ax_iter seq_at(const ax_seq *seq, size_t index);
static void   *seq_last(const ax_seq *seq);
static void   *seq_first(const ax_seq *seq);

static size_t  box_size(const ax_box *box);
static size_t  box_maxsize(const ax_box *box);
static ax_iter box_begin(ax_box *box);
static ax_iter box_end(ax_box *box);
static ax_iter box_rbegin(ax_box *box);
static ax_iter box_rend(ax_box *box);
static const ax_stuff_trait *box_elem_tr(const ax_box *box);

static ax_any *any_copy(const ax_any *any);

static void    citer_move(ax_citer *it, long i);
static void    citer_prev(ax_citer *it);
static void    citer_next(ax_citer *it);
static bool    citer_less(const ax_citer *it1, const ax_citer *it2);
static long    citer_dist(const ax_citer *it1, const ax_citer *it2);

static void    rciter_move(ax_citer *it, long i);
static void    rciter_prev(ax_citer *it);
static void    rciter_next(ax_citer *it);

static void   *iter_get(const ax_iter *it);
static ax_fail iter_set(const ax_iter *it, const void *val);

#ifdef AX_DEBUG
static inline bool iter_if_valid(const ax_citer *it)
{

	const ax_arr *self = it->owner;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;
	ax_byte *ptr = self->arr;
	size_t size = etr->size * self->size;

	return  (ax_citer_norm(it)
		? ((ax_byte *)it->point >= ptr && (ax_byte *)it->point <= ptr + size)
		: ((ax_byte *)it->point >= ptr - etr->size && (ax_byte *)it->point < ptr + size))
		&& ((intptr_t)it->point - (intptr_t)ptr) % etr->size == 0;
}

static inline bool iter_if_have_value(const ax_citer *it)
{
	const ax_arr *self = it->owner;
	const ax_byte *ptr = self->arr;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;
	size_t size = etr->size * self->size;
	return (ax_byte *)it->point >= ptr && (ax_byte *)it->point < ptr + size;
}
#endif

static void citer_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_arr *self = it->owner;
	it->point = (ax_byte*)it->point + (i * (self->seq.env.elem_tr->size));

	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}


static void citer_prev(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_arr *self = it->owner;
	it->point = (ax_byte*)it->point - self->seq.env.elem_tr->size;

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_arr *self = it->owner;
	it->point = (ax_byte*)it->point + self->seq.env.elem_tr->size;

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static bool citer_less(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_PARAM_NULL(it1);
	CHECK_PARAM_NULL(it2);
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it2, iter_if_valid(it2));

	return ax_citer_norm(it1) ? (it1->point < it2->point) : (it1->point > it2->point);
}

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it2, iter_if_valid(it2));

	const ax_arr *self = it1->owner;
	return ((uintptr_t)it2->point - (uintptr_t)it1->point) / self->seq.env.elem_tr->size;
}

static void rciter_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_arr *self = it->owner;
	it->point = (ax_byte*)it->point - (i * (self->seq.env.elem_tr->size));

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void rciter_prev(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_arr *self = it->owner;
	it->point = (ax_byte*)it->point + self->seq.env.elem_tr->size;

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_arr *self = it->owner;
	it->point = (ax_byte*)it->point - self->seq.env.elem_tr->size;

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void *iter_get(const ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);
	CHECK_ITERATOR_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));

	const ax_arr *self = it->owner;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;
	return etr->link ? *(void**) it->point : it->point;
}

static ax_fail iter_set(const ax_iter *it, const void *val)
{
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));

	const ax_arr *self = it->owner;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;

	etr->free(it->point);

	const void *pval = etr->link ? &val : val;
	ax_fail fail = val
		? etr->copy(it->point, pval, etr->size)
		: etr->init(it->point, etr->size);
	if (fail) {
		return true;
	}
	
	return false;
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	return NULL;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	const ax_arr *self = (const ax_arr*)box;
	return self->size;
}

static size_t box_maxsize(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	const ax_arr *self = (const ax_arr*)box;
	return self->size;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_arr *self = (ax_arr *) box;
	ax_iter it = {
		.owner = (void*)box,
		.point = self->arr,
		.tr = &ax_arr_tr.box.iter
	};
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_arr *self = (ax_arr *) box;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;

	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)self->arr + self->size  * etr->size,
		.tr = &ax_arr_tr.box.iter
	};
	return it;
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_arr *self = (ax_arr *) box;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;

	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)self->arr + (self->size - 1) * etr->size,
		.tr = &ax_arr_tr.box.riter
	};
	return it;
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_arr *self = (ax_arr *) box;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;

	ax_iter it = {
		.owner = (void *)box,
		.point = (ax_byte *)self->arr - etr->size,
		.tr = &ax_arr_tr.box.riter
	};
	return it;
}

static const ax_stuff_trait *box_elem_tr(const ax_box *box)
{
	ax_arr_r self_r = { .box = (ax_box*)box };
	return self_r.seq->env.elem_tr;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_arr *self = (ax_arr *) seq;
	const ax_stuff_trait *etr = self->seq.env.elem_tr;
	size_t size = self->size * etr->size;
	ax_byte *ptr = self->arr;

	if (size == 0)
		return;

	size_t left = 0, right = size - etr->size;
	while (right - left > etr->size) {
		seq->env.elem_tr->swap(ptr + left, ptr + right,
				seq->env.elem_tr->size);
		left += etr->size;
		right -= etr->size;
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(ax_r(seq, seq).box));

	ax_arr *self = (ax_arr *) seq;
	self->size = size;
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(ax_cr(seq, seq).box));

	ax_arr *self = (ax_arr *) seq;
	const ax_stuff_trait *etr = seq->env.elem_tr;

	ax_iter it = {
		.owner = self,
		.tr = &ax_arr_tr.box.iter,
		.point =  (ax_byte *)self->arr + index * etr->size
	};
	return it;
}

static void *seq_last(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ax_assert(ax_box_size(ax_cr(seq, seq).box) > 0, "empty");

	ax_arr_cr self_r = { .seq = seq };
	const ax_stuff_trait *etr = self_r.arr->seq.env.elem_tr;
	return (ax_byte *)self_r.arr->arr + (etr->size  - 1) * self_r.arr->size;
}

static void *seq_first(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ax_assert(ax_box_size(ax_cr(seq, seq).box) > 0, "empty");
	
	ax_arr_cr self_r = { .seq = seq };

	return (ax_byte *)self_r.arr->arr;
}

const ax_seq_trait ax_arr_tr =
{
	.box = {
		.any = {
			.one = {
				.name = AX_ARR_NAME,
				.free = NULL,
			},
			.dump = NULL,
			.copy = any_copy,
			.move = NULL 

		},
		.iter = {
			.ctr = {
				.norm = true,
				.type = AX_IT_RAND,
				.move = citer_move,
				.next = citer_next,
				.prev = citer_prev,
				.less = citer_less,
				.dist = citer_dist,
			},
			.get = iter_get,
			.set = iter_set,
			.erase = NULL,
		},
		.riter = {
			.ctr = {
				.norm = false,
				.type = AX_IT_RAND,
				.move = rciter_move,
				.next = rciter_next,
				.prev = rciter_prev,
				.less = citer_less,
				.dist = citer_dist,
			},
			.get = iter_get,
			.set = iter_set,
			.erase = NULL,
		},

		.size = box_size,
		.maxsize = box_maxsize,
		.elem_tr = box_elem_tr,

		.begin = box_begin,
		.end = box_end,
		.rbegin = box_rbegin,
		.rend = box_rend,

		.clear = NULL,

	},
	.push = NULL,
	.pop = NULL,
	.pushf = NULL,
	.popf = NULL,
	.invert = seq_invert,
	.trunc = seq_trunc,
	.at = seq_at,
	.last = seq_last,
	.first = seq_first,
	.insert = NULL,
};


ax_arr_r ax_arr_make(ax_arr *arr, ax_base *base, const ax_stuff_trait *elem_tr, void *ptr, size_t size)
{
	CHECK_PARAM_NULL(base);
	CHECK_PARAM_NULL(elem_tr);
	CHECK_PARAM_NULL(elem_tr->copy);
	CHECK_PARAM_NULL(elem_tr->equal);
	CHECK_PARAM_NULL(elem_tr->free);
	CHECK_PARAM_NULL(elem_tr->init);
	CHECK_PARAM_NULL(elem_tr->move);
	CHECK_PARAM_NULL(elem_tr->swap);

	ax_arr arr_init = {
		.seq = {
			.tr = &ax_arr_tr,
			.env = {
				.one = {
					.base = base,
					.scope = { NULL }
				},
				.elem_tr = elem_tr
			},
		},
		.arr = ptr,
		.size = size
		
	};

	memcpy(arr, &arr_init, sizeof arr_init);
	return (ax_arr_r) { .arr = arr };
}
