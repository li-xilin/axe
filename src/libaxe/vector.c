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

#include <axe/vector.h>
#include <axe/base.h>
#include <axe/def.h>
#include <axe/pool.h>
#include <axe/scope.h>
#include <axe/any.h>
#include <axe/iter.h>
#include <axe/debug.h>
#include <axe/vail.h>
#include <axe/stuff.h>
#include <axe/error.h>
#include <axe/mem.h>
#include <axe/buff.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "check.h"

#undef free

#define MIN_SIZE

struct ax_vector_st
{
	ax_seq _seq;
	ax_one_env one_env;
	ax_buff *buff;
};

static ax_fail     seq_push(ax_seq *seq, const void *val);
static ax_fail     seq_pop(ax_seq *seq);
static void        seq_invert(ax_seq *seq);
static ax_fail     seq_trunc(ax_seq *seq, size_t size);
static ax_iter     seq_at(ax_seq *seq, size_t index);
static ax_fail     seq_insert(ax_seq *seq, ax_iter *it, const void *val);

static size_t      box_size(const ax_box *box);
static size_t      box_maxsize(const ax_box *box);
static ax_iter     box_begin(const ax_box *box);
static ax_iter     box_end(const ax_box *box);
static ax_iter     box_rbegin(const ax_box *box);
static ax_iter     box_rend(const ax_box *box);
static void        box_clear(ax_box *box);
static const ax_stuff_trait *box_elem_tr(const ax_box *box);

static ax_any     *any_copy(const ax_any *any);
static ax_any     *any_move(ax_any *any);

static void        one_free(ax_one *one);

static void        citer_move(ax_citer *it, long i);
static void        citer_prev(ax_citer *it);
static void        citer_next(ax_citer *it);
static ax_bool     citer_less(const ax_citer *it1, const ax_citer *it2);
static long        citer_dist(const ax_citer *it1, const ax_citer *it2);

static void        rciter_move(ax_citer *it, long i);
static void        rciter_prev(ax_citer *it);
static void        rciter_next(ax_citer *it);

static void       *iter_get(const ax_iter *it);
static void        iter_erase(ax_iter *it);
static ax_fail     iter_set(const ax_iter *it, const void *val);

static const ax_any_trait any_trait;
static const ax_box_trait box_trait;
static const ax_seq_trait seq_trait;
static const ax_iter_trait reverse_iter_trait;
static const ax_iter_trait iter_trait;

#ifdef AX_DEBUG
static inline ax_bool iter_if_valid(const ax_citer *it)
{

	const ax_vector *vector = it->owner;
	const ax_stuff_trait *etr = vector->_seq.elem_tr;
	ax_byte *ptr = ax_buff_ptr(vector->buff);
	size_t size = ax_buff_size(vector->buff, NULL);

	return  (ax_citer_norm(it)
		? ((ax_byte *)it->point >= ptr && (ax_byte *)it->point <= ptr + size)
		: ((ax_byte *)it->point >= ptr - etr->size && (ax_byte *)it->point < ptr + size))
		&& ((intptr_t)it->point - (intptr_t)ptr) % etr->size == 0;
}

static inline ax_bool iter_if_have_value(ax_iter it)
{

	ax_vector *vector = it.owner;
	ax_byte *ptr = ax_buff_ptr(vector->buff);
	size_t size = ax_buff_size(vector->buff, NULL);
	return (ax_byte *)it.point >= ptr && (ax_byte *)it.point < ptr + size;
}
#endif

static void citer_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_seq *seq = it->owner;
	it->point = (ax_byte*)it->point + (i * (seq->elem_tr->size));
	ax_assert(iter_if_valid(it), "iterator boundary exceed");
}


static void citer_prev(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	citer_move(it, -1);
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	citer_move(it, 1);
}

static void *iter_get(const ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	ax_vector *vector = (ax_vector *) it->owner;
	const ax_stuff_trait *etr = vector->_seq.elem_tr;
	return etr->link ? *(void**) it->point : it->point;
}

static ax_bool citer_less(const ax_citer *it1, const ax_citer *it2)
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

	const ax_vector *vector = it1->owner;
	return ((uintptr_t)it2->point - (uintptr_t)it1->point) / vector->_seq.elem_tr->size;
}

static void rciter_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_seq *seq = it->owner;
	it->point = (ax_byte*)it->point - (i * (seq->elem_tr->size));
	ax_assert(iter_if_valid(it), "iterator boundary exceed");
}

static void rciter_prev(ax_citer *it)
{
	rciter_move(it, -1);
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	rciter_move(it, 1);
}

static ax_fail iter_set(const ax_iter *it, const void *val)
{

	CHECK_PARAM_VALIDITY(it, iter_if_have_value(*it));

	ax_vector *vector = (ax_vector_r) { (void*)it->owner }.vector;
	const ax_stuff_trait *etr = vector->_seq.elem_tr;
	CHECK_PARAM_VALIDITY(it, (ax_byte *)it->point >= (ax_byte *)ax_buff_ptr(vector->buff)
			&& (ax_byte *)it->point < (ax_byte *)ax_buff_ptr(vector->buff) + ax_buff_size(vector->buff, NULL));

	ax_base *base = ax_one_base(it->owner);
	ax_pool *pool = ax_base_pool(base);
	 
	etr->free(it->point);

	const void *pval = etr->link ? &val : val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, it->point, pval, etr->size)
		: etr->init(pool, it->point, etr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}

	
	return ax_false;
}

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ax_vector_r role = { .one = one };
	ax_scope_detach(one);
	box_clear(role.box);
	ax_one_free(ax_r(buff, role.vector->buff).one);
	ax_pool_free(one);
}

static void any_dump(const ax_any *any, int ind)
{
	printf("not implemented\n");
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_vector_r role = { .any = (ax_any*)any };
	ax_base *base = ax_one_base(role.one);
	ax_pool *pool = ax_base_pool(base);
	ax_vector *new_vector = NULL;
	ax_buff *new_buff = NULL;

	new_vector = ax_pool_alloc(pool, (sizeof(ax_vector)));
	if (!new_vector) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}
	memcpy(new_vector, role.vector, sizeof(ax_vector));
	new_buff = (ax_buff *)ax_any_copy(ax_r(buff, role.vector->buff).any);
	if (!new_buff)
		goto fail;

	new_vector->buff = new_buff;

	new_vector->one_env.sindex = 0;
	new_vector->one_env.scope = NULL;
	ax_scope_attach(ax_base_local(base), ax_r(vector, new_vector).one);
	return ax_r(vector, new_vector).any;
fail:
	ax_pool_free(new_vector);
	ax_one_free(ax_r(buff, new_buff).one);
	return NULL;
}

static ax_any *any_move(ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_vector *src_vec = (ax_vector*)any;
	ax_base *base = ax_one_base(ax_r(vector, src_vec).one);
	ax_pool *pool = ax_base_pool(base);
	ax_buff *dst_buff= NULL;
	ax_vector *dst_vec = NULL;

	dst_vec = ax_pool_alloc(pool, (sizeof(ax_vector)));
	if (!dst_vec) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}
	memcpy(dst_vec, src_vec, sizeof(ax_vector));

	dst_buff = (ax_buff *)ax_any_move(ax_r(buff, src_vec->buff).any);
	if (!dst_buff)
		goto fail;

	dst_vec->buff = dst_buff;

	dst_vec->one_env.sindex = 0;
	dst_vec->one_env.scope = NULL;
	ax_scope_attach(ax_base_local(base), ax_r(vector, dst_vec).one);

	return ax_r(vector, dst_vec).any;
fail:
	ax_pool_free(dst_vec);
	ax_one_free(ax_r(buff, dst_buff).one);
	return NULL;

}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	const ax_vector *vector = (const ax_vector*)box;
	return ax_buff_size(vector->buff, NULL) / vector->_seq.elem_tr->size;
}

static size_t box_maxsize(const ax_box *box)
{
	return 0xFFFFFF;
}

static ax_iter box_begin(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r role = { .box = (ax_box*)box };
	ax_iter it = {
		.owner = (void*)box,
		.point = ax_buff_ptr(role.vector->buff),
		.tr = &iter_trait
	};
	return it;
}

static ax_iter box_end(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r role = { .box = (ax_box*)box};
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)ax_buff_ptr(role.vector->buff) + ax_buff_size(role.vector->buff, NULL),
		.tr = &iter_trait
	};
	return it;
	
}

static ax_iter box_rbegin(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r role = { .box = (ax_box*)box};
	ax_iter it = {
		.owner = (void*)box,
		.point = (ax_byte *)ax_buff_ptr(role.vector->buff) + ax_buff_size(role.vector->buff, NULL) - role.seq->elem_tr->size,
		.tr = &reverse_iter_trait
	};
	return it;
}

static ax_iter box_rend(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_r role = { .box = (ax_box*)box};
	ax_iter it = {
		.owner = (void *)box,
		.point = (ax_byte *)ax_buff_ptr(role.vector->buff) - role.seq->elem_tr->size,
		.tr = &reverse_iter_trait
	};
	return it;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(ax_iter_c(it)));
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(*it));

	ax_vector *vector = (ax_vector_r) { (void*)it->owner }.vector;
	const ax_stuff_trait *etr = vector->_seq.elem_tr;
	ax_byte *ptr = ax_buff_ptr(vector->buff);
	size_t size = ax_buff_size(vector->buff, NULL);
	 
	etr->free(it->point);
	ax_byte *end = ptr + size - etr->size;
	for (ax_byte *p = it->point ; p < end ; p += etr->size) {
		etr->move(p, p + etr->size, etr->size);
	}

	size_t shift = (ax_byte*)it->point - ptr;

	ax_buff_adapt(vector->buff, size - etr->size);
	if(!ax_iter_norm(it))
		it->point = (ax_byte *)ax_buff_ptr(vector->buff) + shift - etr->size;
}

static const ax_stuff_trait *box_elem_tr(const ax_box *box)
{
	ax_vector_r role = { .box = (ax_box*)box };
	return role.seq->elem_tr;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ax_vector *vector = (ax_vector *)box;

	const ax_stuff_trait *etr = vector->_seq.elem_tr;
	ax_byte *ptr = ax_buff_ptr(vector->buff);
	size_t size = ax_buff_size(vector->buff, NULL);

	ax_byte *end = ptr + size;
	for (ax_byte *p = ptr ; p < end ; p += etr->size) {
		etr->free(p);
	}
	ax_buff_adapt(vector->buff, 0);
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner == seq && iter_if_valid(ax_iter_c(it)));

	ax_vector *vector = (ax_vector *) seq;
	const ax_stuff_trait *etr = seq->elem_tr;
	ax_base *base = ax_one_base(ax_r(vector, vector).one);
	ax_pool *pool = ax_base_pool(base);
	ax_byte *ptr = ax_buff_ptr(vector->buff);
	size_t size = ax_buff_size(vector->buff, NULL);

	long offset = (ax_byte *)it->point - ptr; //backup offset before realloc

	if (ax_buff_adapt(vector->buff, size + etr->size))
		return ax_true;

	ptr = ax_buff_ptr(vector->buff);
	it->point = ptr + offset; //restore offset

	void *ins = ax_iter_norm(it) ? it->point : ((ax_byte*)it->point + etr->size);
	void *end = ptr + size;
	for (ax_byte *p = end ; p != ins ; p -= etr->size) {
		etr->move(p, p - etr->size, etr->size);
	}

	const void *pval = etr->link ? &val : val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, ins, pval, etr->size)
		: etr->init(pool, ins, etr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		ax_buff_resize(vector->buff, size);
		return ax_true;
	}

	if(ax_iter_norm(it))
		it->point = (ax_byte*)it->point + etr->size;
	return ax_false;
}

static ax_fail seq_push(ax_seq *seq, const void *val)
{
	CHECK_PARAM_NULL(seq);

	ax_vector *vector = (ax_vector *) seq;
	const ax_stuff_trait *etr = seq->elem_tr;
	ax_base *base = ax_one_base(ax_r(vector, vector).one);
	ax_pool *pool = ax_base_pool(base);

	size_t size = ax_buff_size(vector->buff, NULL);

	if (ax_buff_adapt(vector->buff, size + etr->size))
		return ax_true;

	const void *pval = seq->elem_tr->link ? &val: val;
	ax_byte *ptr = ax_buff_ptr(vector->buff);

	ax_fail fail = (val != NULL)
		? etr->copy(pool, ptr + size, pval, etr->size)
		: etr->init(pool, ptr + size, etr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		ax_buff_resize(vector->buff, size);
		return ax_true;
	}

	return ax_false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_vector *vector = (ax_vector *) seq;
	const ax_stuff_trait *etr = seq->elem_tr;
	size_t size = ax_buff_size(vector->buff, NULL);
	ax_byte *ptr = ax_buff_ptr(vector->buff);

	if (size == 0) {
		ax_base *base = ax_one_base(ax_r(vector, vector).one);
		ax_base_set_errno(base, AX_ERR_EMPTY);
		return ax_false;
	}
	seq->elem_tr->free(ptr + size - etr->size);

	if (ax_buff_adapt(vector->buff, size - etr->size))
		return ax_true;
	return ax_false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_vector *vector = (ax_vector *) seq;
	const ax_stuff_trait *etr = vector->_seq.elem_tr;
	size_t size = ax_buff_size(vector->buff, NULL);
	ax_byte *ptr = ax_buff_ptr(vector->buff);

	if (size == 0)
		return;

	size_t left = 0, right = size - etr->size;
	while (right - left > etr->size) {
		seq->elem_tr->swap(ptr + left, ptr + right,
				seq->elem_tr->size);
		left += etr->size;
		right -= etr->size;
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(&seq->_box));

	ax_vector_r role = { .seq = (ax_seq*)seq};

	ax_vector *vector = (ax_vector *) seq;
	const ax_stuff_trait *etr = seq->elem_tr;
	size_t old_size = ax_buff_size(vector->buff, NULL);


	size *= etr->size;

	if (size == old_size) 
		return ax_false;

	if (size < old_size) {
		ax_byte *ptr = ax_buff_ptr(vector->buff);
		for (size_t off = size; off <= old_size; off += etr->size) {
			seq->elem_tr->free(ptr + off);
		}
		if (ax_buff_adapt(vector->buff, size))
			return ax_true;
	} else {
		ax_base *base = ax_one_base(role.one);
		ax_pool *pool = ax_base_pool(base);
		if (ax_buff_adapt(vector->buff, size))
			return ax_true;
		ax_byte *ptr = ax_buff_ptr(vector->buff);

		for (size_t off = old_size; off < size ; off += etr->size) {
			seq->elem_tr->init(pool, ptr + off, seq->elem_tr->size);
		}
	}
	return ax_false;
}

static ax_iter seq_at(ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(&seq->_box));


	ax_vector *vector = (ax_vector *) seq;
	const ax_stuff_trait *etr = seq->elem_tr;
	ax_byte *ptr = ax_buff_ptr(vector->buff);

	ax_iter it = {
		.owner = vector,
		.tr = &iter_trait,
		.point =  ptr + index * etr->size
	};
	return it;
}

static const ax_one_trait one_trait =
{
	.name = "one.any.box.seq.vector",
	.free = one_free,
	.envp = offsetof(ax_vector, one_env)
};



static const ax_any_trait any_trait =
{
	.dump = any_dump,
	.copy = any_copy,
	.move = any_move
};

static const ax_box_trait box_trait =
{
	.size = box_size,
	.maxsize = box_maxsize,
	.elem_tr = box_elem_tr,

	.begin = box_begin,
	.end = box_end,
	.rbegin = box_rbegin,
	.rend = box_rend,

	.clear = box_clear,
};

static const ax_seq_trait seq_trait =
{
	.push = seq_push,
	.pop = seq_pop,
	.invert = seq_invert,
	.trunc = seq_trunc,
	.at = seq_at,
	.insert = seq_insert,
};

static const ax_iter_trait iter_trait =
{
	.ctr = {
		.norm = ax_true,
		.type = AX_IT_RAND,
		.move = citer_move,
		.next = citer_next,
		.prev = citer_prev,
		.less = citer_less,
		.dist = citer_dist,
	},
	.get = iter_get,
	.set = iter_set,
	.erase = iter_erase,
};

static const ax_iter_trait reverse_iter_trait =
{
	.ctr = {
		.norm = ax_false,
		.type = AX_IT_RAND,
		.move = rciter_move,
		.next = rciter_next,
		.prev = rciter_prev,
		.less = citer_less,
		.dist = citer_dist,
	},
	.get = iter_get,
	.set = iter_set,
	.erase = iter_erase,
};

ax_seq *__ax_vector_construct(ax_base *base,const ax_stuff_trait *elem_tr)
{
	CHECK_PARAM_NULL(base);
	CHECK_PARAM_NULL(elem_tr);
	CHECK_PARAM_NULL(elem_tr->copy);
	CHECK_PARAM_NULL(elem_tr->equal);
	CHECK_PARAM_NULL(elem_tr->free);
	CHECK_PARAM_NULL(elem_tr->init);
	CHECK_PARAM_NULL(elem_tr->move);
	CHECK_PARAM_NULL(elem_tr->swap);

	ax_buff *buff = NULL;
	ax_seq *seq = NULL;

	seq = ax_pool_alloc(ax_base_pool(base), sizeof(ax_vector));
	if (!seq) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	buff = (ax_buff *)__ax_buff_construct(base);
	if (!buff)
		goto fail;

	ax_vector vec_init = {
		._seq = {
			._box = {
				._any = {
					._one = {
						.base = base,
						.tr = &one_trait
					},
					.tr = &any_trait,
				},
				.tr = &box_trait,
			},
			.tr = &seq_trait,
			.elem_tr = elem_tr
		},
		.one_env = {
			.scope = NULL,
			.sindex = 0
		},
		.buff = buff
	};

	memcpy(seq, &vec_init, sizeof vec_init);
	return seq;
fail:
	ax_pool_free(seq);
	ax_one_free(ax_r(buff, buff).one);
	return NULL;
}

ax_vector_r ax_vector_create(ax_scope *scope, const ax_stuff_trait *elem_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(elem_tr);

	ax_base *base = ax_one_base(ax_r(scope, scope).one);
	ax_vector_r role = { .seq = __ax_vector_construct(base, elem_tr) };
	if (!role.one)
		return role;
	ax_scope_attach(scope, role.one);
	return role;
}

void *ax_vector_buffer(ax_vector *vector)
{
	CHECK_PARAM_NULL(vector);

	return ax_buff_ptr(vector->buff);
}
