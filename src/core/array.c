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

#include "ax/array.h"
#include "ax/def.h"
#include "ax/iter.h"
#include "ax/debug.h"
#include "ax/mem.h"
#include "ax/trait.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "check.h"

#undef free

#define MIN_SIZE
#define ELEM_SIZE(r) ax_trait_size(ax_class_data(self.ax_box).elem_tr)

static const char *one_name(const ax_one *one);

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

static ax_any *any_copy(const ax_any *any);

static void    citer_move(ax_citer *it, long i);
static void    citer_prev(ax_citer *it);
static void    citer_next(ax_citer *it);
static bool    citer_less(const ax_citer *it1, const ax_citer *it2);
static long    citer_dist(const ax_citer *it1, const ax_citer *it2);

static void    rciter_move(ax_citer *it, long i);
static void    rciter_prev(ax_citer *it);
static void    rciter_next(ax_citer *it);

static void   *citer_get(const ax_citer *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);

#ifndef NDEBUG
static inline bool iter_if_valid(const ax_citer *it)
{

	ax_array_cr self = { it->owner };
	ax_byte *ptr = self.ax_array->array;
	size_t size = ELEM_SIZE(self) * self.ax_array->size;

	return  (ax_citer_norm(it)
		? ((ax_byte *)it->point >= ptr && (ax_byte *)it->point <= ptr + size)
		: ((ax_byte *)it->point >= ptr - ELEM_SIZE(self) && (ax_byte *)it->point < ptr + size))
		&& ((intptr_t)it->point - (intptr_t)ptr) % ELEM_SIZE(self) == 0;
}

static inline bool iter_if_have_value(const ax_citer *it)
{
	ax_array_cr self = { it->owner };
	const ax_byte *ptr = self.ax_array->array;
	size_t size = ELEM_SIZE(self) * self.ax_array->size;
	return (ax_byte *)it->point >= ptr && (ax_byte *)it->point < ptr + size;
}
#endif

static void citer_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_array_cr self = { it->owner };
	it->point = (ax_byte*)it->point + (i * ELEM_SIZE(self));

	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}


static void citer_prev(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_array_cr self = { it->owner };
	it->point = (ax_byte*)it->point - ELEM_SIZE(self);

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_array_cr self = { it->owner };
	it->point = (ax_byte*)it->point + ELEM_SIZE(self);

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

	const ax_array_cr self = { it1->owner };
	return ((uintptr_t)it2->point - (uintptr_t)it1->point) / ELEM_SIZE(self);
}

static void rciter_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_array_cr self = { it->owner };
	it->point = (ax_byte*)it->point - (i * ELEM_SIZE(self));

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void rciter_prev(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_array_cr self = { it->owner };
	it->point = (ax_byte*)it->point + ELEM_SIZE(self);

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_array_cr self = { it->owner };
	it->point = (ax_byte*)it->point - ELEM_SIZE(self);

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void *citer_get(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);
	CHECK_ITERATOR_VALIDITY(it, iter_if_have_value(it));

	return it->point;
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_cc(it)));

	const ax_array_cr self = { it->owner };
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	ax_trait_free(etr, it->point);
	if (ax_trait_copy_or_init(etr, it->point, val, ap))
		return true;
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

	const ax_array *self = (const ax_array*)box;
	return self->size;
}

static size_t box_maxsize(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	const ax_array *self = (const ax_array*)box;
	return self->size;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_array *self = (ax_array *) box;
	ax_iter it = {
		.owner = (void*)box,
		.point = self->array,
		.tr = &ax_array_tr.ax_box.iter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_array_cr self = { .ax_box = box };
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)self.ax_array->array + self.ax_array->size  * ELEM_SIZE(self),
		.tr = &ax_array_tr.ax_box.iter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_array_r self = { box };
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)self.ax_array->array + (self.ax_array->size - 1) * ELEM_SIZE(self),
		.tr = &ax_array_tr.ax_box.riter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_array_cr self = { box };
	ax_iter it = {
		.owner = (void *)box,
		.point = (ax_byte *)self.ax_array->array - ELEM_SIZE(self),
		.tr = &ax_array_tr.ax_box.riter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_array_r self = { seq };
	size_t size = self.ax_array->size * ELEM_SIZE(self);
	ax_byte *ptr = self.ax_array->array;

	if (size == 0)
		return;

	size_t left = 0, right = size - ELEM_SIZE(self);
	while (right - left > ELEM_SIZE(self)) {
		ax_memswp(ptr + left, ptr + right, ELEM_SIZE(self));
		left += ELEM_SIZE(self);
		right -= ELEM_SIZE(self);
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(ax_r(ax_seq, seq).ax_box));

	ax_array *self = (ax_array *) seq;
	self->size = size;
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(ax_cr(ax_seq, seq).ax_box));

	ax_array_r self = { .ax_seq = (ax_seq *) seq };
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;

	ax_iter it = {
		.owner = self.ax_one,
		.tr = &ax_array_tr.ax_box.iter,
		.etr = etr,
		.point =  (ax_byte *)self.ax_array->array + index * ELEM_SIZE(self),
	};
	return it;
}

static void *seq_last(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ax_assert(ax_box_size(ax_cr(ax_seq, seq).ax_box) > 0, "empty");

	ax_array_cr self = { .ax_seq = seq };
	return (ax_byte *)self.ax_array->array + (ELEM_SIZE(self) - 1) * self.ax_array->size;
}

static void *seq_first(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ax_assert(ax_box_size(ax_cr(ax_seq, seq).ax_box) > 0, "empty");
	
	ax_array_cr self = { .ax_seq = seq };

	return (ax_byte *)self.ax_array->array;
}

static void one_free(ax_one *one)
{
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, ax_array);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_seq_cr self = { .ax_any = any };
	return ax_seq_dump(self.ax_seq);
}

const ax_seq_trait ax_array_tr =
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
			.type = AX_IT_RAND,
			.move = citer_move,
			.next = citer_next,
			.prev = citer_prev,
			.less = citer_less,
			.dist = citer_dist,
			.get = citer_get,
			.set = iter_set,
			.erase = NULL,
			.norm = true,
		},
		.riter = {
			.type = AX_IT_RAND,
			.move = rciter_move,
			.next = rciter_next,
			.prev = rciter_prev,
			.less = citer_less,
			.dist = citer_dist,
			.get = citer_get,
			.set = iter_set,
			.erase = NULL,
			.norm = false,
		},

		.size = box_size,
		.maxsize = box_maxsize,

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


ax_array_r ax_array_make(ax_array *array, const ax_trait *elem_tr, void *ptr, size_t size)
{
	CHECK_PARAM_NULL(elem_tr);

	ax_array array_init = {
		.ax_seq = {
			.tr = &ax_array_tr,
			.env = {
				.ax_box = {
					.ax_any = {
						.ax_one = {
							.scope = { NULL }
						},
					},
					.elem_tr = elem_tr
				},
			},
		},
		.array = ptr,
		.size = size
		
	};

	memcpy(array, &array_init, sizeof array_init);
	return (ax_array_r) { .ax_array = array };
}
