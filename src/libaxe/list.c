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

#undef free

struct node_st
{
	struct node_st *pre;
	struct node_st *next;
	ax_byte data[];
};

struct ax_list_st
{
	ax_seq _seq;
	struct node_st *head;
	size_t size;
	size_t capacity;
};

static ax_fail     seq_push(ax_seq *seq, const void *val);
static ax_fail     seq_pop(ax_seq *seq);
static ax_fail     seq_pushf(ax_seq *seq, const void *val);
static ax_fail     seq_popf(ax_seq *seq);
static ax_fail     seq_trunc(ax_seq *seq, size_t size);
static ax_iter     seq_at(ax_seq *seq, size_t index);
static ax_fail     seq_insert(ax_seq *seq, ax_iter *it, const void *val);

static size_t      box_size(const ax_box *box);
static size_t      box_maxsize(const ax_box *box);
static ax_iter     box_begin(ax_box *box);
static ax_iter     box_end(ax_box *box);
static ax_iter     box_rbegin(ax_box *box);
static ax_iter     box_rend(ax_box *box);
static void        box_clear(ax_box *box);
static const ax_stuff_trait *box_elem_tr(const ax_box* box);

static ax_any     *any_copy(const ax_any *any);
static ax_any     *any_move(ax_any *any);

static void        one_free(ax_one *one);

static void        citer_prev(ax_citer *it);
static void        citer_next(ax_citer *it);
static ax_bool     citer_less(const ax_citer *it1, const ax_citer *it2);
static long        citer_dist(const ax_citer *it1, const ax_citer *it2);

static void        rciter_prev(ax_citer *it);
static void        rciter_next(ax_citer *it);
static ax_bool     rciter_less(const ax_citer *it1, const ax_citer *it2);
static long        rciter_dist(const ax_citer *it1, const ax_citer *it2);

static void       *iter_get(const ax_iter *it);
static ax_fail     iter_set(const ax_iter *it, const void *val);
static void        iter_erase(ax_iter *it);

static void citer_prev(ax_citer *it)
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

static void citer_next(ax_citer *it)
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
	const ax_stuff_trait *etr = list->_seq.env.elem_tr;
	struct node_st *node = it->point;
	return etr->link ? *(void**) node->data : node->data;
}

static ax_bool citer_less(const ax_citer *it1, const ax_citer *it2)
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

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
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

static void rciter_prev(ax_citer *it)
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

static void rciter_next(ax_citer *it)
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

static ax_bool rciter_less(const ax_citer *it1, const ax_citer *it2)
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

static long rciter_dist(const ax_citer *it1, const ax_citer *it2)
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

static ax_fail iter_set(const ax_iter *it, const void *val)
{

	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	struct node_st *node = it->point;
	ax_list *list = (ax_list *) it->owner;
	const ax_stuff_trait *etr = list->_seq.env.elem_tr;

	ax_base *base = ax_one_base(it->owner);
	ax_pool *pool = ax_base_pool(base);
	 
	etr->free(node->data);
	
	const void *pval = etr->link ? &pval : val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, node->data, pval, etr->size)
		: etr->init(pool, node->data, etr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		ax_pool_free(node);
		return ax_true;
	}

	return ax_false;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_list *list = ax_r(list, it->owner).list;
	struct node_st *node = it->point;
	//it->point = node->next; // ?
	it->point = ax_iter_norm(it) ? node->next : node->pre; 
	if (list->size == 1)
		list->head = NULL;
	else {
		if (list->head == node)
			list->head = node->next;
		node->pre->next = node->next;
		node->next->pre = node->pre;
	}

	const ax_stuff_trait *etr = list->_seq.env.elem_tr;
	etr->free(node->data);
	ax_pool_free(node);
}



static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_list_r self_r = { .one = one };
	ax_scope_detach(one);
	box_clear(self_r.box);
	ax_pool_free(one);
}

static void any_dump(const ax_any *any, int ind)
{
	printf("not implemented\n");
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_list_cr self_r = { .any = any };
	const ax_stuff_trait *etr = self_r.seq->env.elem_tr;

	ax_base *base = ax_one_base(self_r.one);
	ax_list_r new_r = { .seq = __ax_list_construct(base, etr) };
	if (new_r.one == NULL) {
		return NULL;
	}


	ax_citer it = ax_box_cbegin(self_r.box);
	ax_citer end = ax_box_cend(self_r.box);
	while (!ax_citer_equal(&it, &end)) {
		const void *pval = ax_citer_get(&it);
		const void *val = etr->link ? *(const void**)pval : pval;
		if (ax_seq_push(new_r.seq, val)) {
			ax_one_free(new_r.one);
			return NULL;
		}
		ax_citer_next(&it);
	}

	new_r.list->_seq.env.one.sindex = 0;
	new_r.list->_seq.env.one.scope = NULL;
	ax_scope_attach(ax_base_local(base), new_r.one);
	return (ax_any*)new_r.any;
}

static ax_any *any_move(ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_list_r self_r = { .any = any };

	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);
	ax_list *new = ax_pool_alloc(pool, (sizeof(ax_list)));
	if (new == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	memcpy(new, self_r.list, sizeof(ax_list));

	self_r.list->head = NULL;
	self_r.list->size = 0;

	new->_seq.env.one.sindex = 0;
	new->_seq.env.one.scope = NULL;
	ax_scope_attach(ax_base_local(base), ax_r(list, new).one);
	return (ax_any*)new;}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list_r self_r = { .box = (ax_box*)box};
	return self_r.list->size;
}

static size_t box_maxsize(const ax_box *box)
{
	return 0xFF;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list_r self_r = { .box = box };
	ax_iter it = {
		.owner = box,
		.point = self_r.list->head,
		.tr = &ax_list_tr.box.iter
	};
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = box,
		.point = NULL,
		.tr = &ax_list_tr.box.iter
	};
	return it;
	
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list_r self_r = { .box = box };
	struct node_st *right_end = self_r.list->head ? self_r.list->head->pre : NULL;
	ax_iter it = {
		.owner = (void *)box,
		.point = right_end,
		.tr = &ax_list_tr.box.riter,
	};
	return it;
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = box,
		.point = NULL,
		.tr = &ax_list_tr.box.riter,
	};
	return it;
}

static const ax_stuff_trait *box_elem_tr(const ax_box* box)
{
	ax_list_r self_r = { .box = (ax_box*)box };
	return self_r.seq->env.elem_tr;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_list *list = (ax_list *)box;

	if (list->size == 0)
		return;

	const ax_stuff_trait *etr = ax_r(list, list).seq->env.elem_tr;
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

	ax_list_r self_r = { .seq = seq };
	CHECK_PARAM_VALIDITY(it, (it->point ? !!self_r.list->head : 1));

	const ax_stuff_trait *etr = self_r.seq->env.elem_tr;
	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);

	struct node_st *node = ax_pool_alloc(pool, sizeof(struct node_st) + etr->size);
	if (node == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}

	const void *pval = etr->link ? &pval : val;
	ax_fail fail = (val != NULL)
		? etr->copy(pool, node->data, pval, etr->size)
		: etr->init(pool, node->data, etr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		ax_pool_free(node);
		return ax_true;
	}

	if (self_r.list->head) {

		if (ax_iter_norm(it)) {
			struct node_st *old = (it->point) ? it->point : self_r.list->head;
			node->pre = old->pre;
			node->next = old;
			old->pre->next = node;
			old->pre = node;
		} else {
			struct node_st *old = (it->point) ? it->point : self_r.list->head->pre;
			node->pre = old;
			node->next = old->next;
			old->next->pre = node;
			old->next = node;
		}
	} else {
		node->pre = node;
		node->next = node;
	}

	if ((ax_iter_norm(it) && it->point == self_r.list->head)
			|| (!ax_iter_norm(it) && it->point == NULL))
		self_r.list->head = node;


	self_r.list->size ++;
	
	it->point = ax_iter_norm(it) ? node->next : node->pre;
	return ax_false;
}

inline static struct node_st *make_node(ax_pool *pool, const ax_stuff_trait *etr, const void *val)
{
	struct node_st *node = NULL;
	node = ax_pool_alloc(pool, sizeof(struct node_st) + etr->size);
	if (!node)
		goto fail;

	const void *pval = etr->link ? &val : val;

	ax_fail fail = (val != NULL)
		? etr->copy(pool, node->data, pval, etr->size)
		: etr->init(pool, node->data, etr->size);
	if (fail)
		goto fail;

	return node;
fail:
	ax_pool_free(node);
	return NULL;
}

static ax_fail seq_push(ax_seq *seq, const void *val)
{
	CHECK_PARAM_NULL(seq);

	ax_list_r self_r = { .seq = seq };

	const ax_stuff_trait *etr = seq->env.elem_tr;
	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);

	struct node_st *node = make_node(pool, etr, val);
	if (!node) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		ax_pool_free(node);
		return ax_true;
	}

	if (self_r.list->head) {
		node->pre = self_r.list->head->pre;
		node->next = self_r.list->head;
		self_r.list->head->pre->next = node;
		self_r.list->head->pre = node;
	} else {
		node->pre = node;
		node->next = node;
		self_r.list->head = node;
	}

	self_r.list->size ++;

	return ax_false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_list *list = (ax_list *)seq;
	ax_base *base = ax_one_base(ax_r(list, list).one);
	if (list->size == 0)
	{
		ax_base_set_errno(base, AX_ERR_EMPTY);
		return ax_false;
	}
	struct node_st *node =  list->head->pre;
	if (list->size > 1) {
		node->pre->next = node->next;
		node->next->pre = node->pre;
	} else {
		list->head = NULL;
	}
	seq->env.elem_tr->free(node->data);
	ax_pool_free(node);

	list->size --;
	return ax_false;
}

static ax_fail seq_pushf(ax_seq *seq, const void *val)
{
	CHECK_PARAM_NULL(seq);

	ax_list_r self_r = { .seq = seq };

	const ax_stuff_trait *etr = seq->env.elem_tr;
	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);

	struct node_st *node = make_node(pool, etr, val);
	if (!node) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		ax_pool_free(node);
		return ax_true;
	}

	if (self_r.list->head) {
		node->pre = self_r.list->head->pre;
		node->next = self_r.list->head;
		self_r.list->head->pre->next = node;
		self_r.list->head->pre = node;
		self_r.list->head = node;
	} else {
		node->pre = node;
		node->next = node;
		self_r.list->head = node;
	}

	self_r.list->size ++;

	return ax_false;
}

static ax_fail seq_popf(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_list *list = (ax_list *)seq;
	ax_base *base = ax_one_base(ax_r(list, list).one);
	if (list->size == 0)
	{
		ax_base_set_errno(base, AX_ERR_EMPTY);
		return ax_false;
	}
	struct node_st *node =  list->head;
	if (list->size > 1) {
		list->head = node->next;
		node->pre->next = node->next;
		node->next->pre = node->pre;
	} else {
		list->head = NULL;
	}
	seq->env.elem_tr->free(node->data);
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
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(ax_r(seq, seq).box));

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

static ax_iter seq_at(ax_seq *seq, size_t index) /* Could optimized to mean time O(n/4) */
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(ax_r(seq, seq).box));

	ax_list *list = (ax_list *)seq;
	struct node_st *cur = list->head;
	ax_iter it = {
		.owner = seq,
		.tr = &ax_list_tr.box.iter,
		.point = cur
	};

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

const ax_seq_trait ax_list_tr =
{
	.box = {
		.any = {
			.one = {
				.name = AX_LIST_NAME,
				.free = one_free,
			},
			.dump = any_dump,
			.copy = any_copy,
			.move = any_move
		},
		.iter = {
			.ctr = {
				.norm = ax_true,
				.type = AX_IT_RAND,
				.move = NULL,
				.next = citer_next,
				.prev = citer_prev,
				.less = citer_less,
				.dist = citer_dist,
			},
			.get = iter_get,
			.set = iter_set,
			.erase = iter_erase
		},
		.riter = {
			.ctr = {
				.norm = ax_false,
				.type = AX_IT_RAND,
				.move = NULL,
				.prev = rciter_prev,
				.next = rciter_next,
				.less = rciter_less,
				.dist = rciter_dist,
			},
			.get = iter_get,
			.set = iter_set,
			.erase = iter_erase
		},

		.size = box_size,
		.maxsize = box_maxsize,

		.begin = box_begin,
		.end = box_end,
		.rbegin = box_rbegin,
		.rend = box_rend,

		.clear = box_clear,
		.elem_tr = box_elem_tr

	},
	.push = seq_push,
	.pop = seq_pop,
	.pushf = seq_pushf,
	.popf = seq_popf,
	.invert = seq_invert,
	.trunc = seq_trunc,
	.at = seq_at,
	.insert = seq_insert,
};


ax_seq* __ax_list_construct(ax_base *base,const ax_stuff_trait *elem_tr)
{
	CHECK_PARAM_NULL(base);
	CHECK_PARAM_NULL(elem_tr);

	CHECK_PARAM_NULL(elem_tr->init);
	CHECK_PARAM_NULL(elem_tr->free);
	CHECK_PARAM_NULL(elem_tr->copy);

	ax_list_r self_r = { ax_pool_alloc(ax_base_pool(base), sizeof(ax_list)) };
	if (self_r.list == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	ax_list list_init = {
		._seq = {
			.tr = &ax_list_tr,
			.env = {
				.one = {
					.base = base,
					.scope = NULL,
					.sindex = 0
				},
				.elem_tr = elem_tr
			},
		},
		.head = NULL,
		.size = 0
	};
	memcpy(self_r.list, &list_init, sizeof list_init);
	return self_r.seq;
}

ax_list_r ax_list_create(ax_scope *scope, const ax_stuff_trait *elem_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(elem_tr);

	ax_base *base = ax_one_base(ax_r(scope, scope).one);
	ax_list_r self_r = { .seq = __ax_list_construct(base, elem_tr) };
	ax_scope_attach(scope, self_r.one);
	return self_r;
}

ax_list_r ax_list_init(ax_scope *scope, const char *fmt, ...)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(fmt);

	va_list varg;
	va_start(varg, fmt);
	ax_list_r self_r = { .seq = ax_seq_vinit(scope, __ax_list_construct, fmt, varg) };
	va_end(varg);
	return self_r;
}

