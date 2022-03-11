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

#include <ax/vector.h>
#include <ax/def.h>
#include <ax/any.h>
#include <ax/iter.h>
#include <ax/debug.h>
#include <ax/trait.h>
#include <ax/mem.h>
#include <ax/buff.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "ax/class.h"
#include "ax/one.h"
#include "check.h"

#undef free

#define MIN_SIZE

ax_begin_data(vector)
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

static void    rciter_move(ax_citer *it, long i);
static void    rciter_prev(ax_citer *it);
static void    rciter_next(ax_citer *it);

static void   *citer_get(const ax_citer *it);
static void    iter_erase(ax_iter *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);

#ifndef NDEBUG
static inline bool iter_if_valid(const ax_citer *it)
{

	const ax_vector *self = it->owner;
	const ax_trait *etr = self->seq.env.box.elem_tr;
	ax_byte *ptr = ax_buff_ptr(self->buff);
	size_t size = ax_buff_size(self->buff, NULL);

	return  (ax_citer_norm(it)
		? ((ax_byte *)it->point >= ptr && (ax_byte *)it->point <= ptr + size)
		: ((ax_byte *)it->point >= ptr - etr->size && (ax_byte *)it->point < ptr + size))
		&& ((intptr_t)it->point - (intptr_t)ptr) % etr->size == 0;
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

	const ax_vector *self = it->owner;
	it->point = (ax_byte*)it->point + (i * (self->seq.env.box.elem_tr->size));

	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}

static void citer_prev(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_vector *self = it->owner;
	it->point = (ax_byte*)it->point - self->seq.env.box.elem_tr->size;

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_vector *self = it->owner;
	it->point = (ax_byte*)it->point + self->seq.env.box.elem_tr->size;

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

	const ax_vector *self = it1->owner;
	return ((uintptr_t)it2->point - (uintptr_t)it1->point)
		/ self->seq.env.box.elem_tr->size;
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

	const ax_vector *self = it->owner;
	it->point = (ax_byte*)it->point - (i * (self->seq.env.box.elem_tr->size));

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void rciter_prev(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_vector *self = it->owner;
	it->point = (ax_byte*)it->point + self->seq.env.box.elem_tr->size;

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_vector *self = it->owner;
	it->point = (ax_byte*)it->point - self->seq.env.box.elem_tr->size;

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

	ax_vector_r self = { it->owner };
	const ax_trait *etr = self.box->env.elem_tr;
	ax_trait_free(etr, it->point);
	if (ax_trait_copy_or_init(etr, it->point, val, ap)) 
		return true;
	return false;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));

	ax_vector *self = (ax_vector_r) { (void*)it->owner }.vector;
	const ax_trait *etr = self->seq.env.box.elem_tr;
	ax_byte *ptr = ax_buff_ptr(self->buff);
	size_t size = ax_buff_size(self->buff, NULL);
	 
	ax_trait_free(etr, it->point);

	ax_byte *end = ptr + size - etr->size;
	for (ax_byte *p = it->point ; p < end ; p += etr->size)
		memcpy(p, p + etr->size, etr->size);

	size_t shift = (ax_byte*)it->point - ptr;
	(void)ax_buff_adapt(self->buff, size - etr->size);
	if(!ax_iter_norm(it))
		it->point = (ax_byte *)ax_buff_ptr(self->buff) + shift - etr->size;
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_vector_r self = { .one = one };
	box_clear(self.box);
	ax_one_free(ax_r(buff, self.vector->buff).one);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, vector);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_seq_cr self = { .any = any };
	return ax_seq_dump(self.seq);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_vector_r self = { .any = (ax_any*)any };
	ax_vector *new_vector = NULL;
	ax_buff *new_buff = NULL;

	new_vector = malloc(sizeof(ax_vector));
	if (!new_vector) {
		goto fail;
	}
	memcpy(new_vector, self.vector, sizeof(ax_vector));
	new_buff = (ax_buff *)ax_any_copy(ax_r(buff, self.vector->buff).any);
	if (!new_buff)
		goto fail;

	new_vector->buff = new_buff;

	new_vector->seq.env.box.any.one.scope.macro = NULL;
	new_vector->seq.env.box.any.one.scope.micro = 0;
	return ax_r(vector, new_vector).any;
fail:
	free(new_vector);
	ax_one_free(ax_r(buff, new_buff).one);
	return NULL;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	const ax_vector *self = (const ax_vector*)box;
	return ax_buff_size(self->buff, NULL) / self->seq.env.box.elem_tr->size;
}

static size_t box_maxsize(const ax_box *box)
{
	const ax_vector *self = (const ax_vector*)box;
	return ax_buff_max(self->buff) / self->seq.env.box.elem_tr->size;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r self = { .box = box };
	ax_iter it = {
		.owner = (void*)box,
		.point = ax_buff_ptr(self.vector->buff),
		.tr = &ax_vector_tr.box.iter,
		.etr = box->env.elem_tr,
	};
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r self = { .box = box };
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)ax_buff_ptr(self.vector->buff) + ax_buff_size(self.vector->buff, NULL),
		.tr = &ax_vector_tr.box.iter,
		.etr = box->env.elem_tr,
	};
	return it;
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r self = { .box = box };
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)ax_buff_ptr(self.vector->buff)
			+ ax_buff_size(self.vector->buff, NULL) - self.seq->env.box.elem_tr->size,
		.tr = &ax_vector_tr.box.riter,
		.etr = box->env.elem_tr,
	};
	return it;
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r self = { .box = box};
	ax_iter it = {
		.owner = (void *)box,
		.point = (ax_byte *)ax_buff_ptr(self.vector->buff) - self.seq->env.box.elem_tr->size,
		.tr = &ax_vector_tr.box.riter,
		.etr = box->env.elem_tr,
	};
	return it;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ax_vector *self = (ax_vector *)box;

	const ax_trait *etr = self->seq.env.box.elem_tr;
	ax_byte *ptr = ax_buff_ptr(self->buff);
	size_t size = ax_buff_size(self->buff, NULL);

	ax_byte *end = ptr + size;
	for (ax_byte *p = ptr ; p < end ; p += etr->size) {
		ax_trait_free(etr, p);
	}
	ax_buff_adapt(self->buff, 0);
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner == seq && iter_if_valid(ax_iter_c(it)));

	ax_vector *self = (ax_vector *) seq;
	const ax_trait *etr = seq->env.box.elem_tr;
	ax_byte *ptr = ax_buff_ptr(self->buff);
	size_t size = ax_buff_size(self->buff, NULL);

	long offset = (ax_byte *)it->point - ptr; //backup offset before realloc

	if (ax_buff_adapt(self->buff, size + etr->size))
		return true;

	ptr = ax_buff_ptr(self->buff);
	it->point = ptr + offset; //restore offset

	void *ins = ax_iter_norm(it) ? it->point : ((ax_byte*)it->point + etr->size);
	void *end = ptr + size;
	for (ax_byte *p = end ; p != ins ; p -= etr->size) {
		memcpy(p, p - etr->size, etr->size);
	}

	if (ax_trait_copy_or_init(etr, ins, val, ap)) {
		ax_buff_resize(self->buff, size);
		return true;
	}

	if(ax_iter_norm(it))
		it->point = (ax_byte*)it->point + etr->size;
	return false;
}

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);

	ax_vector *self = (ax_vector *) seq;
	const ax_trait *etr = seq->env.box.elem_tr;

	size_t size = ax_buff_size(self->buff, NULL);

	if (ax_buff_adapt(self->buff, size + etr->size))
		return true;

	ax_byte *ptr = ax_buff_ptr(self->buff);

	if (ax_trait_copy_or_init(etr, ptr + size, val, ap)) {
		ax_buff_resize(self->buff, size);
		return true;
	}

	return false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_vector *self = (ax_vector *) seq;
	const ax_trait *etr = seq->env.box.elem_tr;
	size_t size = ax_buff_size(self->buff, NULL);
	ax_byte *ptr = ax_buff_ptr(self->buff);

	if (size == 0) {
		return false;
	}
	ax_trait_free(etr, ptr + size - etr->size);

	if (ax_buff_adapt(self->buff, size - etr->size))
		return true;
	return false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_vector *self = (ax_vector *) seq;
	const ax_trait *etr = seq->env.box.elem_tr;
	size_t size = ax_buff_size(self->buff, NULL);
	ax_byte *ptr = ax_buff_ptr(self->buff);

	if (size == 0)
		return;

	size_t left = 0, right = size - etr->size;
	while (right - left > etr->size) {
		ax_memswp(ptr + left, ptr + right, seq->env.box.elem_tr->size);
		left += etr->size;
		right -= etr->size;
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(ax_r(seq, seq).box));

	ax_vector *self = (ax_vector *) seq;
	const ax_trait *etr = seq->env.box.elem_tr;
	size_t old_size = ax_buff_size(self->buff, NULL);


	size *= etr->size;

	if (size == old_size) 
		return false;

	if (size < old_size) {
		ax_byte *ptr = ax_buff_ptr(self->buff);
		for (size_t off = size; off <= old_size; off += etr->size) {
			ax_trait_free(etr, ptr + off);
		}
		if (ax_buff_adapt(self->buff, size))
			return true;
	} else {
		if (ax_buff_adapt(self->buff, size))
			return true;
		ax_byte *ptr = ax_buff_ptr(self->buff);

		for (size_t off = old_size; off < size ; off += etr->size) {
			ax_trait_init(etr, ptr + off, NULL); //FIXME: maybe init failed
		}
	}
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(ax_cr(seq, seq).box));

	ax_vector *self = (ax_vector *) seq;
	const ax_trait *etr = seq->env.box.elem_tr;
	ax_byte *ptr = ax_buff_ptr(self->buff);

	ax_iter it = {
		.owner = self,
		.point =  ptr + index * etr->size,
		.tr = &ax_vector_tr.box.iter,
		.etr = seq->env.box.elem_tr,
	};
	return it;
}

static void *seq_last(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ax_assert(ax_box_size(ax_cr(seq, seq).box) > 0, "empty");
	ax_vector_cr self = { .seq = seq };
	return (ax_byte *)ax_buff_ptr(self.vector->buff)
		+ ax_buff_size(self.vector->buff, NULL)
		- self.vector->seq.env.box.elem_tr->size;
}

static void *seq_first(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_assert(ax_box_size(ax_cr(seq, seq).box) > 0, "empty");
	ax_vector_cr self = { .seq = seq };
	return (ax_byte *)ax_buff_ptr(self.vector->buff);
}

const ax_seq_trait ax_vector_tr =
{
	.box = {
		.any = {
			.one = {
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
			.less = citer_less,
			.dist = citer_dist,
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
			.less = citer_less,
			.dist = citer_dist,
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
	CHECK_PARAM_NULL(elem_tr->copy);
	CHECK_PARAM_NULL(elem_tr->free);

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
		.seq = {
			.tr = &ax_vector_tr,
			.env.box.elem_tr = elem_tr,
		},
		.buff = buff,
	};

	memcpy(seq, &vec_init, sizeof vec_init);
	return seq;
fail:
	free(seq);
	ax_one_free(ax_r(buff, buff).one);
	return NULL;
}

void *ax_vector_buffer(ax_vector *vector)
{
	CHECK_PARAM_NULL(vector);

	return ax_buff_ptr(vector->buff);
}

