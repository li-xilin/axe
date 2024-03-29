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

#include "ax/vector.h"
#include "ax/def.h"
#include "ax/iter.h"
#include "ax/trait.h"
#include "ax/mem.h"
#include "ax/buff.h"
#include "check.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define MIN_SIZE
#define ELEM_SIZE(b) ax_trait_size(ax_class_data(b.ax_box).elem_tr)
#undef free

ax_concrete_begin(ax_vector)
	ax_buff *buff;
ax_end;

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap);
static ax_fail seq_pop(ax_seq *seq);
static void    seq_invert(ax_seq *seq);
static ax_fail seq_trunc(ax_seq *seq, size_t size);
static ax_iter seq_at(const ax_seq *seq, size_t index);
static void   *seq_last(const ax_seq *seq);
static void   *seq_first(const ax_seq *seq);

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap);

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

static void    citer_move(ax_citer *it, long i);
static void    citer_prev(ax_citer *it);
static void    citer_next(ax_citer *it);
static bool    citer_less(const ax_citer *it1, const ax_citer *it2);
static long    citer_dist(const ax_citer *it1, const ax_citer *it2);
ax_box        *citer_box(const ax_citer *it);
static void    iter_erase(ax_iter *it);

static void    rciter_move(ax_citer *it, long i);
static void    rciter_prev(ax_citer *it);
static void    rciter_next(ax_citer *it);
static long    rciter_dist(const ax_citer *it1, const ax_citer *it2);
static void    riter_erase(ax_iter *it);

static void   *citer_get(const ax_citer *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);

#ifndef NDEBUG
static inline bool iter_if_valid(const ax_citer *it)
{

	const ax_vector *self = it->owner;
	const ax_trait *etr = ax_class_data((ax_box *)it->owner).elem_tr;
	ax_byte *ptr = ax_buff_ptr(self->buff);
	size_t size = ax_buff_size(self->buff, NULL);

	return  (ax_citer_norm(it)
		? ((ax_byte *)it->point >= ptr && (ax_byte *)it->point <= ptr + size)
		: ((ax_byte *)it->point >= ptr - ax_trait_size(etr) && (ax_byte *)it->point < ptr + size))
		&& ((intptr_t)it->point - (intptr_t)ptr) % ax_trait_size(etr) == 0;
}

static inline bool iter_if_have_value(const ax_citer *it)
{
	const ax_vector *self = it->owner;
	const ax_byte *ptr = ax_buff_ptr(self->buff);
	size_t size = ax_buff_size(self->buff, NULL);
	return (ax_byte *)it->point >= ptr && (ax_byte *)it->point < ptr + size;
}
#endif

static void citer_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	
	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
	it->point = (ax_byte*)it->point + (i * ax_trait_size(ax_class_data(self.ax_box).elem_tr));

	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}

static void citer_prev(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
	it->point = (ax_byte*)it->point - ELEM_SIZE(self);

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
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

	ax_vector_cr self = AX_R_INIT(ax_one, it1->owner);
	return ((uintptr_t)it2->point - (uintptr_t)it1->point)
		/ ELEM_SIZE(self);
}

ax_box *citer_box(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	return (ax_box *)it->owner;
}

static void rciter_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
	it->point = (ax_byte*)it->point - (i * ELEM_SIZE(self));

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void rciter_prev(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
	it->point = (ax_byte*)it->point + ELEM_SIZE(self);

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static long rciter_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it2, iter_if_valid(it2));

	return - citer_dist(it1, it2);
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
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

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	ax_trait_free(etr, it->point);
	if (ax_trait_copy_or_init(etr, it->point, val, ap)) 
		return true;
	return false;
}


static size_t erase_element(ax_vector_cr self, void *point)
{
	ax_buff *buff = self.ax_vector->buff;

	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	ax_byte *ptr = ax_buff_ptr(buff);
	size_t size = ax_buff_size(buff, NULL);
	 
	ax_trait_free(etr, point);

	ax_byte *end = ptr + size - ELEM_SIZE(self);
	for (ax_byte *p = point ; p < end ; p += ELEM_SIZE(self))
		memcpy(p, p + ELEM_SIZE(self), ELEM_SIZE(self));

	size_t shift = (ax_byte*)point - ptr;
	(void)ax_buff_adapt(buff, size - ELEM_SIZE(self));
	return shift;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
	erase_element(self, it->point);
}

static void riter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));

	ax_vector_cr self = AX_R_INIT(ax_one, it->owner);
	ax_buff *buff = self.ax_vector->buff;
	size_t shift = erase_element(self, it->point);
	it->point = (ax_byte *)ax_buff_ptr(buff) + shift - ELEM_SIZE(self);
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	const ax_vector_r self = AX_R_INIT(ax_one, one);
	box_clear(self.ax_box);
	ax_one_free(ax_r(ax_buff, self.ax_vector->buff).ax_one);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, ax_vector);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_vector_cr self = AX_R_INIT(ax_any, any);
	return ax_seq_dump(self.ax_seq);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_vector_cr self = AX_R_INIT(ax_any, any);
	ax_buff *buff = self.ax_vector->buff;

	ax_vector *new_vector = NULL;
	ax_buff *new_buff = NULL;

	new_vector = malloc(sizeof(ax_vector));
	if (!new_vector) {
		goto fail;
	}
	memcpy(new_vector, self.ax_vector, sizeof(ax_vector));
	new_buff = (ax_buff *)ax_any_copy(ax_r(ax_buff, buff).ax_any);
	if (!new_buff)
		goto fail;

	new_vector->buff = new_buff;

	return ax_r(ax_vector, new_vector).ax_any;
fail:
	free(new_vector);
	ax_one_free(ax_r(ax_buff, new_buff).ax_one);
	return NULL;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_cr self = AX_R_INIT(ax_box, box);
	return ax_buff_size(self.ax_vector->buff, NULL) / ELEM_SIZE(self);
}

static size_t box_maxsize(const ax_box *box)
{
	ax_vector_cr self = AX_R_INIT(ax_box, box);
	return ax_buff_max(self.ax_vector->buff) / ELEM_SIZE(self);
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_cr self = AX_R_INIT(ax_box, box);
	ax_iter it = {
		.owner = (void*)box,
		.point = ax_buff_ptr(self.ax_vector->buff),
		.tr = &ax_vector_tr.ax_box.iter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_cr self = AX_R_INIT(ax_box, box);
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)ax_buff_ptr(self.ax_vector->buff)
			+ ax_buff_size(self.ax_vector->buff, NULL),
		.tr = &ax_vector_tr.ax_box.iter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r self = AX_R_INIT(ax_box, box);
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)ax_buff_ptr(self.ax_vector->buff)
			+ ax_buff_size(self.ax_vector->buff, NULL) - ELEM_SIZE(self),
		.tr = &ax_vector_tr.ax_box.riter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r self = AX_R_INIT(ax_box, box);
	ax_iter it = {
		.owner = box,
		.point = (ax_byte *)ax_buff_ptr(self.ax_vector->buff) - ELEM_SIZE(self),
		.tr = &ax_vector_tr.ax_box.riter,
		.etr = ax_class_data(box).elem_tr,
	};
	return it;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	const ax_vector_cr self = AX_R_INIT(ax_box, box);

	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);
	size_t size = ax_buff_size(self.ax_vector->buff, NULL);

	ax_byte *end = ptr + size;
	for (ax_byte *p = ptr ; p < end ; p += ELEM_SIZE(self)) {
		ax_trait_free(etr, p);
	}
	ax_buff_adapt(self.ax_vector->buff, 0);
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner == seq && iter_if_valid(ax_iter_c(it)));

	const ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);
	size_t size = ax_buff_size(self.ax_vector->buff, NULL);

	long offset = (ax_byte *)it->point - ptr; //backup offset before realloc

	if (ax_buff_adapt(self.ax_vector->buff, size + ELEM_SIZE(self)))
		return true;

	ptr = ax_buff_ptr(self.ax_vector->buff);
	it->point = ptr + offset; //restore offset

	void *ins = ax_iter_norm(it) ? it->point : ((ax_byte*)it->point + ELEM_SIZE(self));
	void *end = ptr + size;
	for (ax_byte *p = end ; p != ins ; p -= ELEM_SIZE(self)) {
		memcpy(p, p - ELEM_SIZE(self), ELEM_SIZE(self));
	}

	if (ax_trait_copy_or_init(etr, ins, val, ap)) {
		ax_buff_resize(self.ax_vector->buff, size);
		return true;
	}

	if(ax_iter_norm(it))
		it->point = (ax_byte*)it->point + ELEM_SIZE(self);
	return false;
}

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);

	const ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;

	size_t size = ax_buff_size(self.ax_vector->buff, NULL);

	if (ax_buff_adapt(self.ax_vector->buff, size + ELEM_SIZE(self)))
		return true;

	ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);

	if (ax_trait_copy_or_init(etr, ptr + size, val, ap)) {
		ax_buff_resize(self.ax_vector->buff, size);
		return true;
	}

	return false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	const ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	size_t size = ax_buff_size(self.ax_vector->buff, NULL);
	ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);

	if (size == 0) {
		return false;
	}
	ax_trait_free(etr, ptr + size - ELEM_SIZE(self));

	if (ax_buff_adapt(self.ax_vector->buff, size - ELEM_SIZE(self)))
		return true;
	return false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	const ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	size_t size = ax_buff_size(self.ax_vector->buff, NULL);
	ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);

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

	const ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	size_t old_size = ax_buff_size(self.ax_vector->buff, NULL);


	size *= ELEM_SIZE(self);

	if (size == old_size) 
		return false;

	if (size < old_size) {
		ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);
		for (size_t off = size; off <= old_size; off += ELEM_SIZE(self)) {
			ax_trait_free(etr, ptr + off);
		}
		if (ax_buff_adapt(self.ax_vector->buff, size))
			return true;
	} else {
		if (ax_buff_adapt(self.ax_vector->buff, size))
			return true;
		ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);

		for (size_t off = old_size; off < size ; off += ELEM_SIZE(self)) {
			ax_trait_init(etr, ptr + off, NULL); //FIXME: maybe init failed
		}
	}
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(ax_cr(ax_seq, seq).ax_box));

	const ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	ax_byte *ptr = ax_buff_ptr(self.ax_vector->buff);

	ax_iter it = {
		.owner = (void *)self.ax_one,
		.point =  ptr + index * ELEM_SIZE(self),
		.tr = &ax_vector_tr.ax_box.iter,
		.etr = etr,
	};
	return it;
}

static void *seq_last(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	ax_assert(ax_box_size(ax_cr(ax_seq, seq).ax_box) > 0, "empty");

	return (ax_byte *)ax_buff_ptr(self.ax_vector->buff)
		+ ax_buff_size(self.ax_vector->buff, NULL) - ELEM_SIZE(self);
}

static void *seq_first(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_assert(ax_box_size(ax_cr(ax_seq, seq).ax_box) > 0, "empty");
	ax_vector_cr self = AX_R_INIT(ax_seq, seq);
	return (ax_byte *)ax_buff_ptr(self.ax_vector->buff);
}

const ax_seq_trait ax_vector_tr =
{
	.ax_box = {
		.ax_any = {
			.ax_one = {
				.free = one_free,
				.name = one_name,
			},
			.dump = any_dump,
			.copy = any_copy,
		},
		.iter = {
			.norm = true,
			.type = AX_IT_RAND,
			.move = citer_move,
			.next = citer_next,
			.prev = citer_prev,
			.dist = citer_dist,
			.less = citer_less,
			.box  = citer_box,
			.get = citer_get,
			.set = iter_set,
			.erase = iter_erase,
		},
		.riter = {
			.norm = false,
			.type = AX_IT_RAND,
			.move = rciter_move,
			.next = rciter_next,
			.prev = rciter_prev,
			.dist = rciter_dist,
			.less = citer_less,
			.get = citer_get,
			.set = iter_set,
			.erase = riter_erase,
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
	.pushf = NULL,
	.popf = NULL,
	.invert = seq_invert,
	.trunc = seq_trunc,
	.at = seq_at,
	.last = seq_last,
	.first = seq_first,
	.insert = seq_insert,
};

ax_seq *__ax_vector_construct(const ax_trait *elem_tr)
{
	CHECK_PARAM_NULL(elem_tr);

	ax_buff *buff = NULL;
	ax_seq *seq = NULL;

	seq = malloc(sizeof(ax_vector));
	if (!seq) {
		goto fail;
	}

	buff = (ax_buff *)__ax_buff_construct();
	if (!buff)
		goto fail;

	ax_vector vec_init = {
		.ax_seq = {
			.tr = &ax_vector_tr,
			.env.ax_box.elem_tr = elem_tr,
		},
		.buff = buff,
	};

	memcpy(seq, &vec_init, sizeof vec_init);
	return seq;
fail:
	free(seq);
	ax_one_free(ax_r(ax_buff, buff).ax_one);
	return NULL;
}

void *ax_vector_buffer(ax_vector *vector)
{
	CHECK_PARAM_NULL(vector);

	return ax_buff_ptr(vector->buff);
}

