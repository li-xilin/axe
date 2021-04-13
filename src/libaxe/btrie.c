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

#include <axe/btrie.h>
#include <axe/avl.h>
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
	struct node_st *parent;
	ax_avl_r sub_r;
	ax_byte *val;
};

struct ax_btrie_st
{
	ax_trie _trie;
	ax_avl_r top_r;
	size_t size;
	size_t capacity;
};

static ax_fail  trie_put(ax_trie *trie, const ax_seq *key, const void *val);
static void    *trie_get(const ax_trie *trie, const ax_seq *key);
static ax_iter  trie_at(const ax_trie *trie, const ax_seq *key);
static ax_bool  trie_exist(const ax_trie *trie, const ax_seq *key);
static void     trie_erase(ax_trie *trie, const ax_seq *key);
static void     trie_rekey(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to);
static ax_iter  trie_root(ax_trie *trie);

static void     trie_it_key(const ax_citer *it, ax_seq *key);
static ax_iter  trie_it_begin(const ax_citer *it);
static ax_iter  trie_it_end(const ax_citer *it);
static ax_iter  trie_it_parent(const ax_citer *it);

static size_t   box_size(const ax_box *box);
static size_t   box_maxsize(const ax_box *box);
static ax_iter  box_begin(ax_box *box);
static ax_iter  box_end(ax_box *box);
static ax_iter  box_rbegin(ax_box *box);
static ax_iter  box_rend(ax_box *box);
static void     box_clear(ax_box *box);
static const ax_stuff_trait *box_elem_tr(const ax_box* box);

static ax_any  *any_copy(const ax_any *any);
static ax_any  *any_move(ax_any *any);

static void     one_free(ax_one *one);

static void     citer_prev(ax_citer *it);
static void     citer_next(ax_citer *it);
static ax_bool  citer_less(const ax_citer *it1, const ax_citer *it2);
static long     citer_dist(const ax_citer *it1, const ax_citer *it2);

static void     rciter_prev(ax_citer *it);
static void     rciter_next(ax_citer *it);
static ax_bool  rciter_less(const ax_citer *it1, const ax_citer *it2);
static long     rciter_dist(const ax_citer *it1, const ax_citer *it2);

static void    *iter_get(const ax_iter *it);
static ax_fail  iter_set(const ax_iter *it, const void *val);
static void     iter_erase(ax_iter *it);

inline static ax_btrie *iter_get_self(const ax_iter *it)
{
	return (ax_btrie *)(((ax_map *)it->owner)->env.one.scope);
}

static void citer_prev(ax_citer *it)
{
	ax_avl_tr.box.iter.ctr.prev(it);
}

static void citer_next(ax_citer *it)
{
	ax_avl_tr.box.iter.ctr.next(it);
}

static ax_bool citer_less(const ax_citer *it1, const ax_citer *it2)
{
	return ax_avl_tr.box.iter.ctr.less(it1, it2);
}

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	return ax_avl_tr.box.iter.ctr.dist(it1, it2);
}

static void rciter_prev(ax_citer *it)
{
	ax_avl_tr.box.riter.ctr.prev(it);
}

static void rciter_next(ax_citer *it)
{
	ax_avl_tr.box.riter.ctr.next(it);
}

static ax_bool rciter_less(const ax_citer *it1, const ax_citer *it2)
{
	return ax_avl_tr.box.riter.ctr.less(it1, it2);
}

static long rciter_dist(const ax_citer *it1, const ax_citer *it2)
{
	return ax_avl_tr.box.riter.ctr.dist(it1, it2);
}

static void *iter_get(const ax_iter *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	ax_btrie *self = iter_get_self(it);
	struct node_st *node = ax_avl_tr.box.iter.get(it);
	void *val = self->_trie.env.val_tr->link ? *(void **)node->val : node->val;
	return val;
}


static ax_fail iter_set(const ax_iter *it, const void *val)
{

	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	return ax_avl_tr.box.iter.set(it, val);

	ax_btrie *self = iter_get_self(it);
	ax_base *base = self->_trie.env.one.base;
	ax_pool *pool = ax_base_pool(base);

	struct node_st *node = ax_avl_tr.box.iter.get(it);
	const ax_stuff_trait *etr = self->_trie.env.val_tr;
	const void *pval = (etr->link) ? &val : val;

	return etr->copy(pool, node->val, pval, etr->size);
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_avl_tr.box.iter.erase(it);
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_btrie_r self_r = { .one = one };
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

#if 0
	ax_btrie_cr self_r = { .any = any };
	const ax_stuff_trait *etr = self_r.seq->env.elem_tr;

	ax_base *base = ax_one_base(self_r.one);
	ax_btrie_r new_r = { .seq = __ax_btrie_construct(base, etr) };
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

	new_r.btrie->_seq.env.one.sindex = 0;
	new_r.btrie->_seq.env.one.scope = NULL;
	ax_scope_attach(ax_base_local(base), new_r.one);
	return (ax_any*)new_r.any;
#endif
	return NULL;
}

static ax_any *any_move(ax_any *any)
{
	CHECK_PARAM_NULL(any);

#if 0
	ax_btrie_r self_r = { .any = any };

	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);
	ax_btrie *new = ax_pool_alloc(pool, (sizeof(ax_btrie)));
	if (new == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	memcpy(new, self_r.btrie, sizeof(ax_btrie));

	self_r.btrie->head = NULL;
	self_r.btrie->size = 0;

	new->_seq.env.one.sindex = 0;
	new->_seq.env.one.scope = NULL;
	ax_scope_attach(ax_base_local(base), ax_r(btrie, new).one);
	return (ax_any*)new;
#endif
	return NULL;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = (ax_box*)box};
	return self_r.btrie->size;
}

static size_t box_maxsize(const ax_box *box)
{
	return 0xFF;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_begin(self_r.btrie->top_r.box);
	it.tr = &ax_trie_tr.box.iter;
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_end(self_r.btrie->top_r.box);
	it.tr = &ax_trie_tr.box.iter;
	return it;
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_begin(self_r.btrie->top_r.box);
	it.tr = &ax_trie_tr.box.riter;
	return it;
	
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	
	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_rend(self_r.btrie->top_r.box);
	it.tr = &ax_trie_tr.box.riter;
	return it;
}

static const ax_stuff_trait *box_elem_tr(const ax_box* box)
{
	ax_btrie_r self_r = { .box = (ax_box*)box };
	return self_r.trie->env.val_tr;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie *btrie = (ax_btrie *)box;

	if (btrie->size == 0)
		return;
	/* ... */

	btrie->size = 0;
}

static ax_fail trie_put(ax_trie *trie, const ax_seq *key, const void *val)
{
	return ax_true;
}

static void *trie_get(const ax_trie *trie, const ax_seq *key)
{
	return NULL;
}

static ax_iter trie_at(const ax_trie *trie, const ax_seq *key)
{
	return (ax_iter) { NULL };
}

static ax_bool trie_exist(const ax_trie *trie, const ax_seq *key)
{
	return ax_false;
}

static void trie_erase(ax_trie *trie, const ax_seq *key)
{
}

static void trie_rekey(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to)
{
}

static ax_iter trie_root(ax_trie *trie)
{
	return (ax_iter) {
		trie->env.one.scope,
		NULL,
		NULL
	};
}

static void trie_it_key(const ax_citer *it, ax_seq *key)
{
}

static ax_iter trie_it_begin(const ax_citer *it)
{
	return (ax_iter) { NULL };
}

static ax_iter trie_it_end(const ax_citer *it)
{
	return (ax_iter) { NULL };
}

static ax_iter trie_it_parent(const ax_citer *it)
{
	return (ax_iter) { NULL };
}

static const ax_stuff_trait node_tr = { 
	.size  = sizeof(struct node_st),
	.equal = NULL,
	.less  = NULL,
	.hash  = NULL,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = ax_false
};

static const ax_trie_trait trie_trait =
{
	

	.box = {
		.any = {
			.one = {
				.name = AX_BTRIE_NAME,
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
				.type = AX_IT_BID,
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

	.put = trie_put,
	.get = trie_get,
	.at = trie_at,
	.exist = trie_exist,
	.rekey = trie_rekey,
	.erase = trie_erase,
	.root = trie_root,
	.it_key = trie_it_key,
	.it_begin = trie_it_begin,
	.it_end = trie_it_end,
	.it_parent = trie_it_parent,
};

ax_trie *__ax_btrie_construct(ax_base *base,const ax_stuff_trait *key_tr, const ax_stuff_trait *val_tr)
{
	CHECK_PARAM_NULL(base);
	CHECK_PARAM_NULL(elem_tr);

	CHECK_PARAM_NULL(val_tr->init);
	CHECK_PARAM_NULL(val_tr->free);
	CHECK_PARAM_NULL(val_tr->copy);

	CHECK_PARAM_NULL(val_tr->init);
	CHECK_PARAM_NULL(val_tr->free);
	CHECK_PARAM_NULL(val_tr->copy);

	ax_map *top = NULL;
	ax_btrie *self = NULL;

	self = ax_pool_alloc(ax_base_pool(base), sizeof(ax_btrie));
	if (!self) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	top = __ax_avl_construct(base, key_tr, &node_tr);
	if (!top) {
		goto fail;
	}
	top->env.one.sindex = SIZE_MAX;
	top->env.one.scope = (ax_scope *)self;

	ax_btrie btrie_init = {
		._trie = {
			.tr = &trie_trait,
			.env = {
				.one = {
					.base = base,
					.scope = NULL,
					.sindex = 0
				},
				.key_tr = key_tr,
				.val_tr = val_tr
			},
		},
		.top_r = { .map = top },
		.size = 0,
		.capacity = 0
	};
	memcpy(self, &btrie_init, sizeof btrie_init);
	return ax_r(btrie, self).trie;
fail:
	ax_one_free(ax_r(map, top).one);
	ax_pool_free(self);
	return NULL;
}

ax_btrie_r ax_btrie_create(ax_scope *scope, const ax_stuff_trait *key_tr, const ax_stuff_trait *val_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(elem_tr);

	ax_base *base = ax_one_base(ax_r(scope, scope).one);
	ax_btrie_r self_r = { .trie = __ax_btrie_construct(base, key_tr, val_tr) };
	ax_scope_attach(scope, self_r.one);
	return self_r;
}


