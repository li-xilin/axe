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

#include <axe/list.h>
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


struct node_st
{
	struct node_st *pre;
	struct node_st *next;
	unsigned char data[0];
};

struct ax_list_st
{
	ax_seq seq;
	ax_one_env one_env;
	struct node_st *head;
	size_t size;
	size_t capacity;
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
static const ax_stuff_trait *box_elem_tr(const ax_box* box);

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


static void iter_move(ax_iter *it, long i)
{
	UNSUPPORTED();
}


static void iter_prev(ax_iter *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);
	ax_list *list = (ax_list *)it->owner;
	struct node_st *node = it->point;
	if (node == NULL) {
		ax_assert(list->head, "iterator boundary exceed");
		it->point = list->head->pre;

	} else {
		ax_assert(node != list->head, "iterator boundary exceed");
		it->point = node->pre;
	}
}

static void iter_next(ax_iter *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);

	ax_list *list = (ax_list *)it->owner;
	struct node_st *node = it->point;
	ax_assert(node != NULL, "iterator boundary exceed");
	if (node->next == list->head) {
		it->point = NULL;

	} else {
		it->point = node->next;
	}
}

static void *iter_get(const ax_iter *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);
	ax_list *list = (ax_list *) it->owner;
	const ax_stuff_trait *etr = list->seq.elem_tr;
	struct node_st *node = it->point;
	return etr->link ? *(void**) node->data : node->data;
}

static ax_fail iter_set(const ax_iter *it, const void *val)
{

	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	struct node_st *node = it->point;
	ax_list *list = (ax_list *) it->owner;
	const ax_stuff_trait *etr = list->seq.elem_tr;

	ax_base *base = ax_one_base(it->owner);
	ax_pool *pool = ax_base_pool(base);
	 
	etr->free(node->data);
	
	const void *pval = etr->link ? &pval : val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, node->data, pval, etr->size)
		: etr->init(pool, node->data, etr->size);
	if (fail) {
		ax_base_set_errno(list->one_env.base, AX_ERR_NOMEM);
		ax_pool_free(node);
		return ax_true;
	}

	return ax_false;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_list *list = ax_cast(list, it->owner).list;
	struct node_st *node = it->point;
	it->point = node->next;
	if (list->size == 1)
		list->head = NULL;
	else {
		if (list->head == node)
			list->head = node->next;
		node->pre->next = node->next;
		node->next->pre = node->pre;
	}

	const ax_stuff_trait *etr = list->seq.elem_tr;
	etr->free(node->data);
	ax_pool_free(node);
}

static ax_bool iter_equal(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	return it1->point == it2->point;
}

static ax_bool iter_less(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	const ax_list *list = it1->owner;

	if (it2->point == NULL)
		return it1->point ? ax_true : ax_false;
	else if (it1->point == NULL)
		return ax_false;

	const struct node_st *node = it1->point;
	node = node->next;
	while (node != list->head) {
		if (node == it2->point)
			return ax_true;
	}
	return ax_false;
}

static long iter_dist(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	const ax_list *list = it1->owner;
	const struct node_st *p = it1->point;
	const struct node_st *q = it2->point;
	long d;

	if (p == NULL && q == NULL)
		return 0;
	if (p) {
		d = 0;
		do {
			if (p == q)
				return d;
			p = p->next;
			d++;
		} while (p != list->head);

		if (q == NULL)
			return d;
	}

	if (q) {
		d = 0;
		do {
			if (p == q)
				return -d;
			q = q->next;
			d++;
		} while (q != list->head);

		if (p == NULL)
			return -d;
	}

	ax_assert(ax_false, "invalid iterator");
	return 0;
}


static void riter_move(ax_iter *it, long i)
{
	UNSUPPORTED();
}

static void riter_prev(ax_iter *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);
	ax_list *list = (ax_list *)it->owner;
	struct node_st *node = it->point;
	if (node == NULL) {
		ax_assert(list->head, "iterator boundary exceed");
		it->point = list->head;

	} else {
		ax_assert(node != list->head->pre, "iterator boundary exceed");
		it->point = node->next;
	}
}

static void riter_next(ax_iter *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr);

	ax_list *list = (ax_list *)it->owner;
	struct node_st *node = it->point;
	ax_assert(node != NULL, "iterator boundary exceed");
	if (node == list->head) {
		it->point = NULL;

	} else {
		it->point = node->pre;
	}
}

static ax_bool riter_less(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	const ax_list *list = it1->owner;

	if (it2->point == NULL)
		return it1->point ? ax_true : ax_false;
	else if (it1->point == NULL)
		return ax_false;

	const struct node_st *node = it1->point;
	node = node->pre;
	while (node != list->head->pre) {
		if (node == it2->point)
			return ax_true;
	}
	return ax_false;
}

static long riter_dist(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	const ax_list *list = it1->owner;
	const struct node_st *p = it1->point;
	const struct node_st *q = it2->point;
	long d;

	if (p == NULL && q == NULL)
		return 0;
	if (p) {
		d = 0;
		do {
			if (p == q)
				return d;
			p = p->pre;
			d++;
		} while (p != list->head->pre);

		if (q == NULL)
			return d;
	}

	if (q) {
		d = 0;
		do {
			if (p == q)
				return - d;
			q = q->pre;
			d++;
		} while (q != list->head->pre);

		if (p == NULL)
			return - d;
	}

	ax_assert(ax_false, "invalid iterator");
	return 0;
}


static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_list_role role = { .one = one };
	ax_scope_detach(one);
	box_clear(role.box);
	ax_pool_free(one);
}

static ax_one_env *one_envp(const ax_one *one)
{
	CHECK_PARAM_NULL(one);

	ax_list_role role = { .one = (ax_one *)one };
	return &role.list->one_env;
}

static void any_dump(const ax_any *any, int ind)
{
	printf("not implemented\n");
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_list *list = (ax_list *)any;
	const ax_stuff_trait *etr = list->seq.elem_tr;

	ax_base *base = ax_one_base(&list->seq.box.any.one);
	ax_list_role new_role = { .seq = __ax_list_construct(base, etr) };
	if (new_role.one == NULL) {
		return NULL;
	}


	ax_iter it = ax_box_begin(&list->seq.box);
	ax_iter end = ax_box_end(&list->seq.box);
	while (!ax_iter_equal(it, end)) {
		const void *pval = ax_iter_get(it);
		const void *val = etr->link ? *(const void**)pval : pval;
		if (ax_seq_push(new_role.seq, val)) {
			ax_one_free(new_role.one);
			return NULL;
		}
		it = ax_iter_next(it);
	}

	new_role.list->one_env.sindex = 0;
	new_role.list->one_env.scope = NULL;
	ax_scope_attach(ax_base_local(base), new_role.one);
	return (ax_any*)new_role.any;
}

static ax_any *any_move(ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_list *list = (ax_list *)any;

	ax_base *base = ax_one_base(&list->seq.box.any.one);
	ax_pool *pool = ax_base_pool(base);
	ax_list *new = ax_pool_alloc(pool, (sizeof(ax_list)));
	if (new == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	memcpy(new, list, sizeof(ax_list));

	list->head = NULL;
	list->size = 0;

	new->one_env.sindex = 0;
	new->one_env.scope = NULL;
	ax_scope_attach(ax_base_local(base), (ax_one*)&new->seq.box.any.one);
	return (ax_any*)new;}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list_role role = { .box = (ax_box*)box};
	return role.list->size;
}

static size_t box_maxsize(const ax_box *box)
{
	return 0xFF;
}

static ax_iter box_begin(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list_role role = { .box = (ax_box*)box };
	ax_iter it = {
		.owner = (void *)box,
		.point = role.list->head,
		.tr = &iter_trait };
	return it;
}

static ax_iter box_end(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = (void *)box,
		.point = NULL,
		.tr = &iter_trait
	};
	return it;
	
}

static ax_iter box_rbegin(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list_role role = { .box = (ax_box*)box};
	struct node_st *right_end = role.list->head ? role.list->head->pre : NULL;
	ax_iter it = {
		.owner = (void *)box,
		.point = right_end,
		.tr = &reverse_iter_trait,
	};
	return it;
}

static ax_iter box_rend(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = (void *)box,
		.point = NULL,
		.tr = &reverse_iter_trait,
	};
	return it;
}

static const ax_stuff_trait *box_elem_tr(const ax_box* box)
{
	ax_list_role role = { .box = (ax_box*)box };
	return role.seq->elem_tr;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list *list = (ax_list *)box;

	if (list->size == 0)
		return;

	const ax_stuff_trait *etr = list->seq.elem_tr;
	struct node_st *node = list->head;
	do {	
		struct node_st *next = node->next;
		etr->free(node->data);
		ax_pool_free(node);
		node = next;
	} while (node != list->head);

	list->head = NULL;
	list->size = 0;
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_list *list = (ax_list *)seq;
	CHECK_PARAM_VALIDITY(it, (it->point ? !!list->head : 1));

	const ax_stuff_trait *etr = list->seq.elem_tr;
	ax_pool *pool = ax_base_pool(list->one_env.base);

	struct node_st *node = ax_pool_alloc(pool, sizeof(struct node_st) + etr->size);
	if (node == NULL) {
		ax_base_set_errno(list->one_env.base, AX_ERR_NOMEM);
		return ax_true;
	}

	const void *pval = etr->link ? &pval : val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, node->data, pval, etr->size)
		: etr->init(pool, node->data, etr->size);
	if (fail) {
		ax_base_set_errno(list->one_env.base, AX_ERR_NOMEM);
		ax_pool_free(node);
		return ax_true;
	}

	if (list->head) {

		if (it->tr->norm) {
			struct node_st *old = (it->point) ? it->point : list->head;
			node->pre = old->pre;
			node->next = old;
			old->pre->next = node;
			old->pre = node;
		} else {
			struct node_st *old = (it->point) ? it->point : list->head->pre;
			node->pre = old;
			node->next = old->next;
			old->next->pre = node;
			old->next = node;
		}
	} else {
		node->pre = node;
		node->next = node;
	}

	if ((it->tr->norm && it->point == list->head)
			|| (!it->tr->norm && it->point == NULL))
		list->head = node;


	list->size ++;
	
	it->point = ax_iter_norm(*it) ? node->next : node->pre;
	return ax_false;
}

static ax_fail seq_push(ax_seq *seq, const void *val)
{
	CHECK_PARAM_NULL(seq);

	ax_list *list = (ax_list *)seq;

	const ax_stuff_trait *etr = list->seq.elem_tr;
	ax_pool *pool = ax_base_pool(list->one_env.base);

	struct node_st *node = ax_pool_alloc(pool, sizeof(struct node_st) + etr->size);
	if (node == NULL) {
		ax_base_set_errno(list->one_env.base, AX_ERR_NOMEM);
		return ax_true;
	}

	const void *pval = etr->link ? &val : val;

	ax_fail fail = (val != NULL)
		? etr->copy(pool, node->data, pval, etr->size)
		: etr->init(pool, node->data, etr->size);
	if (fail) {
		ax_base_set_errno(list->one_env.base, AX_ERR_NOMEM);
		ax_pool_free(node);
		return ax_true;
	}

	if (list->head) {
		node->pre = list->head->pre;
		node->next = list->head;
		list->head->pre->next = node;
		list->head->pre = node;
	} else {
		node->pre = node;
		node->next = node;
		list->head = node;
	}

	list->size ++;

	return ax_false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_list *list = (ax_list *)seq;
	if (list->size == 0)
	{
		ax_base_set_errno(list->one_env.base, AX_ERR_EMPTY);
		return ax_false;
	}
	struct node_st *node =  list->head->pre;
	if (list->size > 1) {
		node->pre->next = node->next;
		node->next->pre = node->pre;
	} else {
		list->head = NULL;
	}
	seq->elem_tr->free(node->data);
	ax_pool_free(node);

	list->size --;
	return ax_false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ax_list *list = (ax_list *)seq;
	if (list->size < 2)
		return;

	list->head->pre->next = NULL; /* Break link circle */

	struct node_st *new_head = list->head;

	struct node_st *cur = list->head->next;
	while (cur) {
		struct node_st *next = cur->next;
		cur->next = new_head;
		new_head->pre = cur;
		new_head = cur;
		cur = next;
	}
	new_head->pre = list->head;
	list->head->next = new_head;

	list->head = new_head;
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(&seq->box));

	ax_list *list = (ax_list *)seq;

	if (size > list->size) {
		size_t npush = size - list->size;
		while (npush--) {
			if (ax_seq_push(seq, NULL)) {
				while (npush++ < size - list->size) {
					ax_seq_pop(seq);
				}
				return ax_true;
			}
		}
	} else {
		size_t npop = list->size - size;
		while (npop--) {
			ax_seq_pop(seq);
		}

	}

	return ax_false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index) /* Could optimized to mean time O(n/4) */
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(&seq->box));

	ax_list *list = (ax_list *)seq;
	struct node_st *cur = list->head;
	ax_iter it = { .owner = (void *)seq, .tr = &iter_trait, .point = cur };

	if (index == 0)
		return it;

	do {
		index --;
		cur = cur->next;
	} while (index && cur != list->head);

	ax_assert(index == 0, "index boundary exceed");
	it.point = cur;
	return it;
}

static const ax_one_trait one_trait =
{
	.name = "one.any.box.seq.list",
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

	.begin = box_begin,
	.end = box_end,
	.rbegin = box_rbegin,
	.rend = box_rend,

	.clear = box_clear,
	.elem_tr = box_elem_tr
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
	.less = iter_less,
	.dist = iter_dist,
	.equal = iter_equal,

	.get = iter_get,
	.set = iter_set,
	.erase = iter_erase
};

static const ax_iter_trait reverse_iter_trait =
{
	.norm = ax_false,
	.type = AX_IT_RAND,

	.move = riter_move,
	.prev = riter_prev,
	.next = riter_next,
	.less = riter_less,
	.dist = riter_dist,
	.equal = iter_equal,

	.get = iter_get,
	.set = iter_set,
	.erase = iter_erase
};

ax_seq* __ax_list_construct(ax_base *base,const ax_stuff_trait *elem_tr)
{
	CHECK_PARAM_NULL(base);
	CHECK_PARAM_NULL(elem_tr);

	CHECK_PARAM_NULL(elem_tr->init);
	CHECK_PARAM_NULL(elem_tr->free);
	CHECK_PARAM_NULL(elem_tr->copy);

	ax_list_role role = { ax_pool_alloc(ax_base_pool(base), sizeof(ax_list)) };
	if (role.list == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	ax_list list_init = {
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
		.head = 0,
		.size = 0
	};
	memcpy(role.list, &list_init, sizeof list_init);
	return role.seq;
}

ax_list_role ax_list_create(ax_scope *scope, const ax_stuff_trait *elem_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(elem_tr);

	ax_list_role role = { .seq = __ax_list_construct(ax_scope_base(scope), elem_tr) };
	ax_scope_attach(scope, role.one);
	return role;
}

ax_list_role ax_list_init(ax_scope *scope, const char *fmt, ...)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(fmt);

	va_list varg;
	va_start(varg, fmt);
	ax_list_role role = { .seq = ax_seq_vinit(scope, __ax_list_construct, fmt, varg) };
	va_end(varg);
	return role;
}

