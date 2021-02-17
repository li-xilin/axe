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

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "check.h"

struct ax_vector_st
{
	ax_seq seq;
	ax_one_env one_env;
	size_t size;
	size_t capacity;
	size_t maxsize;
	ax_byte *buffer;
};

static ax_fail     seq_push(ax_seq *seq, const void *val);
static ax_fail     seq_pop(ax_seq *seq);
static ax_fail     seq_trunc(ax_seq *seq, size_t size);
static ax_iter     seq_at(const ax_seq *seq, size_t index);
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
static ax_one_env *one_envp(const ax_one *one);

static void        iter_move(ax_iter *it, long i);
static void        iter_prev(ax_iter *it);
static void        iter_next(ax_iter *it);
static void       *iter_get(const ax_iter *it);
static ax_fail     iter_set(const ax_iter *it, const void *val);
static void        iter_erase(ax_iter *it);
static ax_bool     iter_equal(const ax_iter *it1, const ax_iter *it2);
static ax_bool     iter_less(const ax_iter *it1, const ax_iter *it2);
static long        iter_dist(const ax_iter *it1, const ax_iter *it2);

static const ax_any_trait any_trait;
static const ax_box_trait box_trait;
static const ax_seq_trait seq_trait;
static const ax_iter_trait reverse_iter_trait;
static const ax_iter_trait iter_trait;

#ifdef AX_DEBUG
static inline ax_bool iter_if_valid(const ax_iter *it)
{
	ax_vector_role role = { (void*)it->owner};
	const ax_stuff_trait *etr = role.vector->seq.elem_tr;
	return  (ax_iter_norm(*it)
		? (it->point >= role.vector->buffer && it->point <= role.vector->buffer + role.vector->size * etr->size)
		: (it->point >= role.vector->buffer - etr->size && it->point < role.vector->buffer + role.vector->size * etr->size))
		&& ((intptr_t)it->point - (intptr_t)role.vector->buffer) % etr->size == 0;
}

static inline ax_bool iter_if_have_value(ax_iter it)
{
	ax_vector_role role = { (void*)it.owner};
	const ax_stuff_trait *etr = role.vector->seq.elem_tr;
	return it.point >= role.vector->buffer && it.point < role.vector->buffer + role.vector->size * etr->size;
}
#endif

static ax_fail realloc_buffer_before_add(ax_vector *vector)
{
	ax_vector_role role = { .vector = vector };
	const ax_stuff_trait *etr = vector->seq.elem_tr;
	if (vector->size == vector->capacity) {
		if(vector->capacity == vector->maxsize) {
			ax_base_set_errno(ax_one_base(role.one), AX_ERR_FULL);
			return ax_true;
		}
		vector->capacity <<= 1;
		vector->capacity |= 1;
		if(vector->capacity > vector->maxsize)
			vector->capacity = vector->maxsize;
		void *ptr = realloc(vector->buffer, vector->capacity * etr->size);
		if (ptr == NULL) {
			ax_base_set_errno(ax_one_base(role.one), AX_ERR_NOMEM);
			return ax_true;
		}
		vector->buffer = ptr;
	}
	return ax_false;
}

static ax_fail realloc_buffer_after_remove(ax_vector *vector)
{
	if (vector->size == vector->capacity >> 2) {
		vector->capacity >>= 1;
		void *ptr = realloc(vector->buffer, vector->capacity * vector->seq.elem_tr->size);
		if (ptr == NULL) {
			ax_assert(ax_false, "failed to realloc memory");
			return ax_true;
		}
		vector->buffer = ptr;
	}
	return ax_false;
}

static void iter_move(ax_iter *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_seq *seq = it->owner;
	it->point = (ax_byte*)it->point + (i * (seq->elem_tr->size));
	ax_assert(iter_if_valid(it), "iterator boundary exceed");
}


static void iter_prev(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	iter_move(it, -1);
}

static void iter_next(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	iter_move(it, 1);
}

static void *iter_get(const ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);
	ax_vector *vector = (ax_vector *) it->owner;
	const ax_stuff_trait *etr = vector->seq.elem_tr;
	return etr->link ? *(void**) it->point : it->point;
}

static ax_fail iter_set(const ax_iter *it, const void *val)
{

	CHECK_PARAM_VALIDITY(it, iter_if_have_value(*it));

	ax_vector *vector = (ax_vector_role) { (void*)it->owner }.vector;
	const ax_stuff_trait *etr = vector->seq.elem_tr;
	CHECK_PARAM_VALIDITY(it, it->point >= vector->buffer && it->point < vector->buffer + vector->size * etr->size);

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

static ax_bool iter_equal(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it2, iter_if_valid(it2));

	return it1->point == it2->point;
}

static ax_bool iter_less(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_PARAM_NULL(it1);
	CHECK_PARAM_NULL(it2);
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it2, iter_if_valid(it2));

	return ax_iter_norm(*it1) ? (it1->point < it2->point) : (it1->point > it2->point);
}

static long iter_dist(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it2, iter_if_valid(it2));

	const ax_vector *vector = it1->owner;
	return ((uintptr_t)it2->point - (uintptr_t)it1->point) / vector->seq.elem_tr->size;
}

static void riter_move(ax_iter *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));

	const ax_seq *seq = it->owner;
	it->point = (ax_byte*)it->point - (i * (seq->elem_tr->size));
	ax_assert(iter_if_valid(it), "iterator boundary exceed");
}

static void riter_prev(ax_iter *it)
{
	riter_move(it, -1);
}

static void riter_next(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	riter_move(it, 1);
}



static void one_free(ax_one *one)
{
	if (!one)
		return;
	ax_vector_role role = { .one = one };
	ax_scope_detach(one);
	box_clear(role.box);
	ax_pool_free(one);
}

static ax_one_env *one_envp(const ax_one *one)
{
	CHECK_PARAM_NULL(one);

	ax_vector_role role = { .one = (ax_one *)one };
	return &role.vector->one_env;
}

static void any_dump(const ax_any *any, int ind)
{
	printf("not implemented\n");
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_vector_role role = { .any = (ax_any*)any };
	ax_base *base = ax_one_base(role.one);
	ax_pool *pool = ax_base_pool(base);
	ax_vector *new = ax_pool_alloc(pool, (sizeof(ax_vector)));
	if (!new) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}
	memcpy(new, role.vector, sizeof(ax_vector));
	new->buffer = malloc(role.seq->elem_tr->size * role.vector->capacity);
	if (!new->buffer) {
		ax_pool_free(new);
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}
	memcpy(new->buffer, role.vector->buffer, role.seq->elem_tr->size * role.vector->size);
	new->one_env.sindex = 0;
	new->one_env.scope = NULL;
	ax_scope_attach(ax_base_local(base), (ax_one*)&new->seq.box.any.one);
	return (ax_any*)new;
}

static ax_any *any_move(ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_vector_role role = { .any = (ax_any*)any };
	ax_base *base = ax_one_base(role.one);
	ax_pool *pool = ax_base_pool(base);
	ax_vector *new = ax_pool_alloc(pool, (sizeof(ax_vector)));
	if (!new) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}
	memcpy(new, role.vector, sizeof(ax_vector));
	new->one_env.sindex = 0;
	new->one_env.scope = NULL;
	ax_scope_attach(ax_base_local(base), (ax_one*)&new->seq.box.any.one);

	role.vector->buffer = NULL;
	role.vector->size = 0;
	role.vector->capacity = 0;

	return (ax_any*)new;

}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	const ax_vector *vector = (const ax_vector*)box;
	return vector->size;
}

static size_t box_maxsize(const ax_box *box)
{
	return 0xFFFFFF;
}

static ax_iter box_begin(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_role role = { .box = (ax_box*)box };
	ax_iter it = { .owner = (void*)box, .point = role.vector->buffer, .tr = &iter_trait };
	return it;
}

static ax_iter box_end(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_role role = { .box = (ax_box*)box};
	ax_iter it = {
		.owner = (void*)box,
		.point = role.vector->buffer + (role.seq->elem_tr->size * role.vector->size),
		.tr = &iter_trait
	};
	return it;
	
}

static ax_iter box_rbegin(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_role role = { .box = (ax_box*)box};
	ax_iter it;
	it.owner = (void *)box;
	it.point = role.vector->buffer + (role.vector->size - 1) * role.seq->elem_tr->size;
	it.tr = &reverse_iter_trait;
	return it;
}

static ax_iter box_rend(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_vector_role role = { .box = (ax_box*)box};
	ax_iter it;
	it.owner = (void *)box;
	it.point = role.vector->buffer - role.seq->elem_tr->size;
	it.tr = &reverse_iter_trait;
	return it;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(*it));

	ax_vector *vector = (ax_vector_role) { (void*)it->owner }.vector;
	const ax_stuff_trait *etr = vector->seq.elem_tr;
	 
	vector->seq.elem_tr->free(it->point);
	ax_byte *end = vector->buffer + (vector->size - 1) * etr->size;
	for (ax_byte *p = it->point ; p < end ; p += etr->size) {
		etr->move(p, p + etr->size, etr->size);
	}

	vector->size --;
	if(!ax_iter_norm(*it))
		it->point = (ax_byte*)it->point - etr->size;
}

static const ax_stuff_trait *box_elem_tr(const ax_box *box)
{
	ax_vector_role role = { .box = (ax_box*)box };
	return role.seq->elem_tr;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ax_vector_role role = { .box = (ax_box*)box};
	const ax_stuff_trait *etr = role.vector->seq.elem_tr;
	ax_byte *end = role.vector->buffer + role.vector->size * etr->size;
	if (role.vector->buffer) {
		for (ax_byte *p = role.vector->buffer ; p < end ; p += etr->size) {
			etr->free(p);
		}
		free(role.vector->buffer);
		role.vector->buffer = NULL;
		role.vector->size = 0;
		role.vector->capacity = 0;
	}
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner == seq && iter_if_valid(it));

	ax_vector *vector = (ax_vector_role) { .seq = seq }.vector;
	const ax_stuff_trait *etr = vector->seq.elem_tr;
	ax_base *base = vector->one_env.base;
	ax_pool *pool = ax_base_pool(base);

	size_t offset = (ax_byte *)it->point - vector->buffer; //backup offset before realloc
	if(realloc_buffer_before_add(vector))
		return ax_true;
	it->point = vector->buffer + offset; //restore offset

	void *ins = ax_iter_norm(*it) ? it->point : ((ax_byte*)it->point + etr->size);
	void *end = vector->buffer + (vector->size) * etr->size;
	for (ax_byte *p = end ; p != ins ; p -= etr->size) {
		etr->move(p, p - etr->size, etr->size);
	}

	const void *pval = etr->link ? &val : val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, ins, pval, etr->size)
		: etr->init(pool, ins, etr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}

	vector->size ++;
	if(ax_iter_norm(*it))
		it->point = (ax_byte*)it->point + etr->size;
	return ax_false;
}

static ax_fail seq_push(ax_seq *seq, const void *val)
{
	CHECK_PARAM_NULL(seq);

	ax_vector_role role = { .seq = seq };
	const ax_stuff_trait *etr = role.vector->seq.elem_tr;
	ax_base *base = ax_one_base(role.one);
	ax_pool *pool = ax_base_pool(base);

	if (realloc_buffer_before_add(role.vector))
		return ax_true;

	const void *pval = seq->elem_tr->link ? &val: val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, role.vector->buffer + (role.vector->size * etr->size), pval, etr->size)
		: etr->init(pool, role.vector->buffer + (role.vector->size * etr->size), etr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}

	role.vector->size ++;

	return ax_false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_vector_role role = { .seq = seq};
	if (role.vector->size == 0) {
		ax_base_set_errno(ax_one_base(role.one), AX_ERR_EMPTY);
		return ax_false;
	}
	role.vector->size --;
	seq->elem_tr->free(role.vector->buffer + role.vector->size  * role.seq->elem_tr->size);
	if(realloc_buffer_after_remove(role.vector))
		return ax_true;
	return ax_false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_vector_role role = { .seq = seq};
	if (role.vector->size == 0)
		return;
	size_t left = 0, right = role.vector->size - 1;
	while (right - left > 1) {
		seq->elem_tr->swap(role.vector->buffer + left * seq->elem_tr->size,
				role.vector->buffer + right * seq->elem_tr->size,
				seq->elem_tr->size);
		left++, right--;
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(&seq->box));

	ax_vector_role role = { .seq = (ax_seq*)seq};
	if (size == role.vector->size) 
		return ax_false;

	size_t capacity = role.vector->capacity;
	if (size < role.vector->size) {
		for (size_t i = size; i <= role.vector->size; i++) {
			seq->elem_tr->free(role.vector->buffer + i * role.seq->elem_tr->size);
		}

		role.vector->size = size;
		if (size == 0)
			capacity = 1;
		else {
			while(capacity > size)
				capacity >>= 1;
			capacity = (capacity << 1) | 1;
		}

		void *alloc_ptr = realloc(role.vector->buffer, capacity * role.seq->elem_tr->size);
		if (alloc_ptr)
			role.vector->buffer = alloc_ptr;
		role.vector->capacity = capacity;
	} else {
		ax_base *base = ax_one_base(role.one);
		ax_pool *pool = ax_base_pool(base);
		while(capacity < size) {
			capacity <<= 1;
			capacity |= 1;
		}
		void *alloc_ptr = realloc(role.vector->buffer, capacity * role.seq->elem_tr->size);
		if (alloc_ptr == NULL) {
			ax_base_set_errno(role.vector->one_env.base, AX_ERR_NOMEM);
			return ax_true;
		}
		role.vector->buffer = alloc_ptr;
		role.vector->capacity = capacity;

		for (size_t i = role.vector->size; i < size; i++) {
			seq->elem_tr->init(pool, role.vector->buffer + i * role.seq->elem_tr->size,
					seq->elem_tr->size);
		}
		role.vector->size = size;
	}
	return ax_false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(&seq->box));

	ax_vector_role role = { .seq = (ax_seq*)seq};
	ax_iter it = {
		.owner = role.vector,
		.tr = &iter_trait,
		.point =  role.vector->buffer + index * role.seq->elem_tr->size
	};
	return it;
}

static const ax_one_trait one_trait =
{
	.name = "one.any.box.seq.vector",
	.free = one_free,
	.envp = one_envp
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
	.norm = ax_true,
	.type = AX_IT_RAND,

	.move = iter_move,
	.next = iter_next,
	.prev = iter_prev,

	.get = iter_get,
	.set = iter_set,
	.erase = iter_erase,

	.equal = iter_equal,
	.less = iter_less,

	.dist = iter_dist,
};

static const ax_iter_trait reverse_iter_trait =
{
	.norm = ax_false,
	.type = AX_IT_RAND,

	.move = riter_move,
	.prev = riter_prev,
	.next = riter_next,

	.get = iter_get,
	.set = iter_set,
	.erase = iter_erase,

	.equal = iter_equal,
	.less = iter_less,

	.dist = iter_dist,
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

	ax_vector_role role = { ax_pool_alloc(ax_base_pool(base), sizeof(ax_vector)) };
	if (role.vector == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	ax_vector vec_init = {
		.seq = {
			.box = {
				.any = {
					.one = {
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
			.base = base,
			.scope = NULL,
			.sindex = 0
		},
		.maxsize = box_trait.maxsize(role.box),
		.buffer = NULL,
		.capacity = 0,
		.size = 0,

	};
	memcpy(role.vector, &vec_init, sizeof vec_init);
	return role.seq;
}

ax_vector_role ax_vector_create(ax_scope *scope, const ax_stuff_trait *elem_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(elem_tr);

	ax_vector_role role = { .seq = __ax_vector_construct(ax_scope_base(scope), elem_tr) };
	ax_scope_attach(scope, role.one);
	return role;
}

void *ax_vector_buffer(ax_vector *vector)
{
	CHECK_PARAM_NULL(vector);

	return vector->buffer;
}
