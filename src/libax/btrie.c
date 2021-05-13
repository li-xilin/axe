/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
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

#include <ax/btrie.h>
#include <ax/avl.h>
#include <ax/list.h>
#include <ax/base.h>
#include <ax/def.h>
#include <ax/pool.h>
#include <ax/scope.h>
#include <ax/any.h>
#include <ax/iter.h>
#include <ax/debug.h>
#include <ax/vail.h>
#include <ax/stuff.h>
#include <ax/error.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include "check.h"

#undef free

struct node_st
{
	ax_avl_r submap_r;
	ax_byte *val;
};

struct ax_btrie_st
{
	ax_trie _trie;
	ax_avl_r root_r;
	size_t size;
	size_t capacity;
};

static ax_fail  trie_put(ax_trie *trie, const ax_seq *key, const void *val);
static void    *trie_get(const ax_trie *trie, const ax_seq *key);
static ax_iter  trie_at(const ax_trie *trie, const ax_seq *key);
static ax_bool  trie_exist(const ax_trie *trie, const ax_seq *key);
static ax_bool  trie_erase(ax_trie *trie, const ax_seq *key);
static ax_bool  trie_prune(ax_trie *trie, const ax_seq *key);
static ax_fail  trie_rekey(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to);

static const void *trie_it_word(const ax_citer *it);
static ax_iter  trie_it_begin(const ax_citer *it);
static ax_iter  trie_it_end(const ax_citer *it);
static ax_bool  trie_it_parent(const ax_citer *it, ax_iter *parent);
static ax_bool  trie_it_valued(const ax_citer *it);

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
static ax_box  *citer_box(const ax_citer *it);

static void    *iter_get(const ax_iter *it);
static ax_fail  iter_set(const ax_iter *it, const void *val);
static void     iter_erase(ax_iter *it);

static int match_key(const ax_btrie *self, const ax_seq *key, ax_citer *it_mismatched, struct node_st **last_node);
static ax_fail node_set_value(ax_btrie *self, struct node_st *node, const void *val);
static ax_bool clean_path(ax_map *last);
static void rec_remove(ax_btrie *self, ax_map *map);
inline static ax_btrie *iter_get_self(const ax_iter *it);

static const ax_stuff_trait node_tr;

inline static ax_btrie *iter_get_self(const ax_iter *it)
{
	return (ax_btrie *)(ax_one_envp(it->owner)->scope.macro);
}


static void citer_prev(ax_citer *it)
{
	ax_avl_tr.box.iter.ctr.prev(it);
}

static void citer_next(ax_citer *it)
{
	ax_avl_tr.box.iter.ctr.next(it);
}

static ax_box *citer_box(const ax_citer *it)
{
	const ax_map *map = it->owner;
	return (ax_box *)map->env.one.scope.macro;
}

inline static void node_set_parent(struct node_st *node, const void *parent)
{
	ax_one_envp(node->submap_r.one)->scope.micro = (uintptr_t)parent;
}

inline static void *node_get_parent(const struct node_st *node)
{
	return (void *)ax_one_envp(node->submap_r.one)->scope.micro;
}

static void *iter_get(const ax_iter *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	ax_btrie *self = iter_get_self(it);
	struct node_st *node = ax_avl_tr.box.iter.get(it);
	void *val = self->_trie.env.val_tr->link
		? *(void **)node->val
		: node->val;
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
	if (node->val) {
		etr->free(node->val);
	} else {
		node->val = ax_pool_alloc(pool, etr->size);
		if (!node->val) {
			ax_base_set_errno(base, AX_ERR_NOMEM);
			return ax_true;
		}
	}

	return etr->copy(pool, node->val, pval, etr->size);
}

#if 0
/* return value : if map is freed */
static ax_bool node_remove_value(ax_btrie *self, struct node_st *node)
{
	if (node->val) {
		const ax_stuff_trait *etr = self->_trie.env.val_tr;
		etr->free(node->val);
		node->val = NULL;
	}
	if (ax_box_size(node->submap_r.box) == 0) {
		ax_avl_tr.box.iter.erase(it);
		if (clean_path(node_get_parent(node))) {
			*it = box_end(ax_r(btrie, self).box);
		}
	}
}
#endif

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_btrie *self = iter_get_self(it);

	struct node_st *node = ax_avl_tr.box.iter.get(it);
	if (node->val) {
		const ax_stuff_trait *etr = self->_trie.env.val_tr;
		etr->free(node->val);
		ax_pool_free(node->val);
		node->val = NULL;
		self->size --;
	}
	if (ax_box_size(node->submap_r.box) == 0) {
		ax_one_free(node->submap_r.one);
		ax_avl_tr.box.iter.erase(it);
		if (clean_path(node_get_parent(node))) {
			*it = box_end(ax_r(btrie, self).box);
		}
	}
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_btrie_r self_r = { .one = one };
	ax_scope_detach(one);
	box_clear(self_r.box);
	ax_one_free(self_r.btrie->root_r.one);
	ax_pool_free(one);
}

static void any_dump(const ax_any *any, int ind)
{
	printf("not implemented\n");
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	// TODO
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

	// TODO
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
	CHECK_PARAM_NULL(box);
	
	ax_btrie_r self_r = { .box = (ax_box*)box};
	return self_r.btrie->capacity;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_begin(self_r.btrie->root_r.box);
	it.tr = &ax_btrie_tr.box.iter;
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_end(self_r.btrie->root_r.box);
	it.tr = &ax_btrie_tr.box.iter;
	return it;
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_begin(self_r.btrie->root_r.box);
	it.tr = &ax_btrie_tr.box.riter;
	return it;
	
}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };
	ax_iter it = ax_box_rend(self_r.btrie->root_r.box);
	it.tr = &ax_btrie_tr.box.riter;
	return it;
}

static const ax_stuff_trait *box_elem_tr(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = (ax_box*)box };
	return self_r.trie->env.val_tr;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };

	if (self_r.btrie->size == 0)
		return;
	
	rec_remove(self_r.btrie, self_r.btrie->root_r.map);
}

void node_dump(ax_map *map, int i)
{
	ax_map_foreach(map, const int *, key, struct node_st *, node) {
		printf("%*c map = %p, parent = %p, key = %d, val = %d\n", i * 4, ' ', (void *)map,  node_get_parent(node), *key, (node->val ? *(int *)node->val : -8));
		node_dump(node->submap_r.map, i+1);
	}
}

void ax_btrie_dump(ax_btrie *btrie)
{
	CHECK_PARAM_NULL(btrie);

	printf("top = %p\n", (void *)btrie->root_r.one->env.scope.micro);
	node_dump(btrie->root_r.map, 1);
}

static int match_key(const ax_btrie *self, const ax_seq *key, ax_citer *it_mismatched, struct node_st **last_node)
{
	size_t match_len = 0;
	ax_map *cur_map = self->root_r.map;

	*last_node = NULL;
	
	ax_citer key_it = ax_box_cbegin(ax_cr(seq, key).box);
	ax_citer key_it_end = ax_box_cend(ax_cr(seq, key).box);

	if (ax_box_size(self->root_r.box) > 0) {
		match_len ++;


		ax_iter root_it = ax_box_begin(ax_r(map, cur_map).box);
		*last_node = ax_iter_get(&root_it);
		cur_map = (*last_node)->submap_r.map;

		while (!ax_citer_equal(&key_it, &key_it_end)) {
			ax_iter find_pos = ax_map_at(cur_map, ax_citer_get(&key_it));
			ax_iter find_end = ax_box_end(ax_r(map, cur_map).box);
			if (ax_iter_equal(&find_pos, &find_end))
				break;
			*last_node = ax_iter_get(&find_pos);
			cur_map = (*last_node)->submap_r.map;
			ax_citer_next(&key_it);
			match_len ++;
		}
	}
	*it_mismatched = key_it;
	return match_len;
}

static ax_fail node_set_value(ax_btrie *self, struct node_st *node, const void *val)
{

	ax_base *base = ax_one_base(ax_r(btrie, self).one);
	ax_pool *pool = ax_base_pool(base);

	const ax_stuff_trait *val_tr = self->_trie.env.val_tr;
	void *value = NULL;

	value = ax_pool_alloc(pool, val_tr->size);
	if (!value) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	const void *pval = val_tr->link ? &val : val;
	ax_fail fail = (val == NULL)
		? val_tr->init(pool, value, val_tr->size)
		: val_tr->copy(pool, value, pval, val_tr->size);
	if (fail) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	if (node->val) {
		val_tr->free(node->val);
		ax_pool_free(node->val);
	} else {
		self->size ++;
	}
	node->val = value;

	return ax_false;
fail:
	ax_pool_free(value);
	return ax_true;
}

static struct node_st *make_path(ax_trie *trie, const ax_seq *key)
{
	ax_assert(trie->env.key_tr == key->env.elem_tr, "invalid element trait for the key");

	ax_btrie_r self_r = { .trie = trie };
	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);

	ax_citer key_it;
	struct node_st *last_node;
	int match_len = match_key(self_r.btrie, key, &key_it, &last_node);

	struct node_st *new_node_tab = NULL;
	void *value = NULL;
	
	size_t ins_count = ax_box_size(ax_cr(seq, key).box) + 1 - match_len;

	new_node_tab = ax_pool_alloc(pool, sizeof(struct node_st) * ins_count);
	if (!new_node_tab) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}
	memset(new_node_tab, 0, sizeof(struct node_st) * ins_count);

	int count = 0;
	for (size_t i = 0; i < ins_count; i++) {
		ax_map *new_submap = __ax_avl_construct(base, self_r.btrie->_trie.env.key_tr, &node_tr);
		new_submap->env.one.scope.macro = self_r.one;
		if (!new_submap) {
			ax_base_set_errno(base, AX_ERR_NOMEM);
			goto fail;
		}
		new_node_tab[i].submap_r.map = new_submap;
		count ++;
	}

	ax_map *cur_map;
	if (match_len == 0) {
		node_set_parent(new_node_tab + 0, self_r.btrie->root_r.map);
		last_node = ax_map_put(self_r.btrie->root_r.map, NULL, new_node_tab + 0);
		cur_map = last_node->submap_r.map;
	} else {
		cur_map = last_node->submap_r.map;
	}

	for (size_t i = !match_len; i < ins_count; i++) {
		node_set_parent(new_node_tab + i, cur_map);
		const void *key = ax_citer_get(&key_it);
		last_node = ax_map_put(cur_map, key, new_node_tab + i);
		if (!last_node)
			goto fail;
		cur_map = new_node_tab[i].submap_r.map;
		ax_citer_next(&key_it);
	}

	ax_pool_free(new_node_tab);
	return last_node;
fail:
	if (new_node_tab) {
		for (size_t i = 0; i < ins_count; i++)
			ax_one_free(new_node_tab[i].submap_r.one);
		ax_pool_free(new_node_tab);
	}
	ax_pool_free(value);
	return NULL;
}

static ax_fail trie_put(ax_trie *trie, const ax_seq *key, const void *val)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);
	ax_assert(trie->env.key_tr == key->env.elem_tr, "invalid element trait for the key");

	ax_btrie_r self_r = { .trie = trie };

	struct node_st *node = make_path(trie, key);
	if (!node)
		return ax_true;
	
	if (node_set_value(self_r.btrie, node, val))
		return ax_true;
	return ax_false;
}

static void *trie_get(const ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	return ax_iter_equal(&it, &end)
		? NULL
		: iter_get(&it);
}

static ax_iter trie_at(const ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_btrie *self = (ax_btrie *) trie;
	if (ax_box_size(ax_cr(map, self->root_r.map).box) == 0)
		return box_end((ax_box *)trie);

	ax_iter it = ax_box_begin(ax_r(map, self->root_r.map).box);

	struct node_st *last_node = ax_iter_get(&it);
	if (ax_box_size(ax_cr(seq, key).box)) {
		ax_box_cforeach(ax_cr(seq, key).box, const void *, word) {
			it = ax_map_at(last_node->submap_r.map, word);
			ax_iter end = ax_box_end(last_node->submap_r.box);
			if (ax_iter_equal(&it, &end))
				return box_end((ax_box *)trie);
			last_node = ax_iter_get(&it);
		}
	}
	it.tr = &ax_btrie_tr.box.iter;
	return it;
}

static ax_bool trie_exist(const ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	if (ax_iter_equal(&it, &end))
		return ax_false;
	struct node_st *node = ax_avl_tr.box.iter.get(&it);
	return !!node->val;

}

static ax_bool clean_path(ax_map *last) {
	ax_map_r parent_r = { .map = last };
	ax_bool retval = ax_false;

	if (last == NULL)
		return retval;

	if (ax_box_size(parent_r.box))
		return retval;

	parent_r.one = (ax_one *)parent_r.one->env.scope.micro;
	if (parent_r.one == NULL)
		return retval;

	while (parent_r.one) {
		if (ax_box_size(parent_r.box) != 1) {
			return retval;
		}
		ax_iter it = ax_box_begin(parent_r.box);
		struct node_st *node = ax_avl_tr.box.iter.get(&it);
		if (node->val)
			return retval;
		
		ax_one_free(node->submap_r.one);
		ax_box_clear(parent_r.box);
		parent_r.one = (ax_one *)parent_r.one->env.scope.micro;
		retval = ax_true;
	}
	return retval;
}

static ax_bool trie_erase(ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	if (ax_iter_equal(&it, &end))
		return ax_false;
	iter_erase(&it);
	return ax_true;
}

static void rec_remove(ax_btrie *self, ax_map *map)
{
	const ax_stuff_trait *val_tr = self->_trie.env.val_tr;
	ax_map_r map_r = ax_r(map, map);
	ax_iter cur = ax_box_begin(map_r.box);
	ax_iter end = ax_box_end(map_r.box);
	while (!ax_iter_equal(&cur, &end)) {
		struct node_st *node = ax_avl_tr.box.iter.get(&cur);
		rec_remove(self, node->submap_r.map);
		if (node->val) {
			val_tr->free(node->val);
			ax_pool_free(node->val);
			self->size--;
		}
		ax_one_free(node->submap_r.one);
		ax_iter_erase(&cur);
	}
}

static ax_bool trie_prune(ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_btrie *self = (ax_btrie *) trie;
	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	if (ax_iter_equal(&it, &end))
		return ax_false;
	struct node_st *node = ax_avl_tr.box.iter.get(&it);
	rec_remove(self, node->submap_r.map);
	iter_erase(&it);
	return ax_true;
}

static ax_fail trie_rekey(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key_from);
	CHECK_PARAM_NULL(key_to);

	//TODO: do not work
	ax_iter it = trie_at(trie, key_from);
	ax_iter end = box_end((ax_box *)trie);
	if (ax_iter_equal(&it, &end))
		return ax_false;

	struct node_st *old_node = ax_avl_tr.box.iter.get(&it);
	if (!old_node->val)
		return ax_false;

	void *value = old_node->val;

	struct node_st *new_node = make_path(trie, key_to);
	if (!new_node)
		return ax_true;

	new_node->val = value;
	old_node->val = NULL;
	iter_erase(&it);

	return ax_true;
}

static const void *trie_it_word(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));

	return ax_avl_tr.itkey(it);
}

static ax_iter trie_it_begin(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));

	struct node_st *node = ax_avl_tr.box.iter.get((ax_iter *)it);
	ax_iter ret = ax_box_begin(node->submap_r.box);
	ret.tr = &ax_btrie_tr.box.iter;
	return ret;
}

static ax_iter trie_it_end(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));

	struct node_st *node = ax_avl_tr.box.iter.get((ax_iter *)it);
	ax_iter ret = ax_box_end(node->submap_r.box);
	ret.tr = &ax_btrie_tr.box.iter;
	return ret;
}

static ax_bool  trie_it_parent(const ax_citer *it, ax_iter *parent)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));
	UNSUPPORTED();

	return ax_false;
}

static ax_bool trie_it_valued(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));

	struct node_st *node = ax_avl_tr.box.iter.get((ax_iter *)it);
	return !!node->val;
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

const ax_trie_trait ax_btrie_tr =
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
				.less = NULL,
				.dist = NULL,
				.box = citer_box
			},
			.get = iter_get,
			.set = iter_set,
			.erase = iter_erase
		},
		.riter = { { NULL } },

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
	.prune = trie_prune,
	.rekey = trie_rekey,
	.erase = trie_erase,
	.it_word = trie_it_word,
	.it_begin = trie_it_begin,
	.it_end = trie_it_end,
	.it_parent = trie_it_parent,
	.it_valued = trie_it_valued
};

ax_trie *__ax_btrie_construct(ax_base *base,const ax_stuff_trait *key_tr, const ax_stuff_trait *val_tr)
{
	CHECK_PARAM_NULL(base);

	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(key_tr->equal);
	CHECK_PARAM_NULL(val_tr->hash);
	CHECK_PARAM_NULL(val_tr->copy);
	CHECK_PARAM_NULL(val_tr->free);

	CHECK_PARAM_NULL(val_tr);
	CHECK_PARAM_NULL(val_tr->copy);
	CHECK_PARAM_NULL(val_tr->free);

	ax_map *root = NULL;
	ax_btrie *self = NULL;

	self = ax_pool_alloc(ax_base_pool(base), sizeof(ax_btrie));
	if (!self) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	root = __ax_avl_construct(base, ax_stuff_traits(AX_ST_NIL), &node_tr);
	if (!root) {
		goto fail;
	}
	root->env.one.scope.macro = ax_r(btrie, self).one;
	root->env.one.scope.micro = 0;

	ax_btrie btrie_init = {
		._trie = {
			.tr = &ax_btrie_tr,
			.env = {
				.one = {
					.base = base,
					.scope = { NULL },
				},
				.key_tr = key_tr,
				.val_tr = val_tr
			},
		},
		.root_r = { .map = root },
		.size = 0,
		.capacity = SIZE_MAX
	};
	memcpy(self, &btrie_init, sizeof btrie_init);
	return ax_r(btrie, self).trie;
fail:
	ax_one_free(ax_r(map, root).one);
	ax_pool_free(self);
	return NULL;
}

ax_btrie_r ax_btrie_create(ax_scope *scope, const ax_stuff_trait *key_tr, const ax_stuff_trait *val_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(val_tr);

	ax_base *base = ax_one_base(ax_r(scope, scope).one);
	ax_btrie_r self_r = { .trie = __ax_btrie_construct(base, key_tr, val_tr) };
	ax_scope_attach(scope, self_r.one);
	return self_r;
}

