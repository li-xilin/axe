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
#include <ax/def.h>
#include <ax/any.h>
#include <ax/iter.h>
#include <ax/debug.h>
#include <ax/trait.h>
#include <ax/dump.h>
#include <ax/class.h>

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

AX_CLASS_STRUCT_ENTRY(btrie)
	ax_avl_r root_r;
	size_t size;
	size_t capacity;
AX_END;

static void    *trie_put(ax_trie *trie, const ax_seq *key, const void *val);
static void    *trie_get(const ax_trie *trie, const ax_seq *key);
static ax_iter  trie_at(const ax_trie *trie, const ax_seq *key);
static bool     trie_exist(const ax_trie *trie, const ax_seq *key, bool *valued);
static bool     trie_erase(ax_trie *trie, const ax_seq *key);
static bool     trie_prune(ax_trie *trie, const ax_seq *key);
static ax_fail  trie_rekey(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to);

static const void *trie_it_word(const ax_citer *it);
static ax_iter  trie_it_begin(const ax_citer *it);
static ax_iter  trie_it_end(const ax_citer *it);
static bool     trie_it_parent(const ax_citer *it, ax_iter *parent);
static bool     trie_it_valued(const ax_citer *it);
static bool     trie_it_clean(ax_iter *it);

static size_t   box_size(const ax_box *box);
static size_t   box_maxsize(const ax_box *box);
static ax_iter  box_begin(ax_box *box);
static ax_iter  box_end(ax_box *box);
static ax_iter  box_rbegin(ax_box *box);
static ax_iter  box_rend(ax_box *box);
static void     box_clear(ax_box *box);

static ax_any  *any_copy(const ax_any *any);
static ax_dump *any_dump(const ax_any *any);

static void     one_free(ax_one *one);

static void     citer_prev(ax_citer *it);
static void     citer_next(ax_citer *it);
static ax_box  *citer_box(const ax_citer *it);

static void    *citer_get(const ax_citer *it);
static ax_fail  iter_set(const ax_iter *it, const void *val);
static void     iter_erase(ax_iter *it);

static int match_key(const ax_btrie *self, const ax_seq *key, ax_citer *it_mismatched, struct node_st **last_node);
static ax_fail node_set_value(ax_btrie *self, struct node_st *node, const void *val);
static void node_free_value(ax_btrie *btrie, struct node_st *node);
static bool clean_path(ax_map *last);
static void rec_remove(ax_btrie *self, ax_map *map);
inline static ax_btrie *iter_get_self(const ax_iter *it);

static const ax_trait node_tr;

inline static ax_btrie *iter_get_self(const ax_iter *it)
{
	return (ax_btrie *)(ax_one_envp(it->owner)->scope.macro);
}

static void citer_prev(ax_citer *it)
{
	ax_avl_tr.box.iter.prev(it);
}

static void citer_next(ax_citer *it)
{
	ax_avl_tr.box.iter.next(it);
}

static ax_box *citer_box(const ax_citer *it)
{
	const ax_map *map = it->owner;
	return (ax_box *)map->env.box.any.one.scope.macro;
}

inline static void node_set_parent(struct node_st *node, const void *parent)
{
	ax_one_envp(node->submap_r.one)->scope.micro = (uintptr_t)parent;
}

inline static void *node_get_parent(const struct node_st *node)
{
	return (void *)ax_one_envp(node->submap_r.one)->scope.micro;
}

static void *citer_get(const ax_citer *it)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	struct node_st *node = ax_avl_tr.box.iter.get(it);
	return node->val;
}

static ax_fail iter_set(const ax_iter *it, const void *val)
{
	CHECK_ITERATOR_VALIDITY(it, it->owner && it->tr && it->point);

	return ax_avl_tr.box.iter.set(it, val);

	ax_btrie *self = iter_get_self(it);

	struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_cc(it));
	const ax_trait *etr = self->trie.env.box.elem_tr;
	if (node->val) {
		ax_trait_free(etr, node->val);
	} else { 
		node->val = malloc(etr->size);
		if (!node->val)
			return true;
	}

	return ax_trait_copy(etr, node->val, val);
}

static void node_free_value(ax_btrie *btrie, struct node_st *node)
{
	if (node->val) {
		const ax_trait *etr = btrie->trie.env.box.elem_tr;
		ax_trait_free(etr, node->val);
		free(node->val);
		node->val = NULL;
		btrie->size --;
	}
}

static bool trie_it_clean(ax_iter *it)
{
	struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_c(it));
	ax_iter cur = ax_box_begin(node->submap_r.box),
		end = ax_box_end(node->submap_r.box);

	while (!ax_iter_equal(&cur, &end)) {
		if (!trie_it_valued(ax_iter_c(&cur))) {
			if (!trie_it_clean(&cur))
				ax_iter_next(&cur);
		} else
			ax_iter_next(&cur);
	}

	if (!node->val && !ax_box_size(node->submap_r.box)) {
		ax_one_free(node->submap_r.one);
		ax_avl_tr.box.iter.erase(it);
		return true;
	}
	return false;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_btrie *self = iter_get_self(it);

	//TODO: Change ax_avl_tr.box.iter.get to ax_iter_get
	struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_c(it));
	node_free_value(self, node);
	trie_it_clean(it);
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_btrie_r self_r = { .one = one };
	box_clear(self_r.box);
	ax_avl_tr.box.any.one.free(self_r.btrie->root_r.one);
	free(one);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_trie_cr self = { .any = any };
	return ax_trie_dump(self.trie);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	// TODO
#if 0
	ax_btrie_cr self_r = { .any = any };
	const ax_trait *etr = self_r.seq->env.box.elem_tr;

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

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_btrie_r self_r = { .box = box };

	if (self_r.btrie->size == 0)
		return;
	
	rec_remove(self_r.btrie, self_r.btrie->root_r.map);
	assert(self_r.btrie->size == 0);
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
	const ax_trait *vtr = self->trie.env.box.elem_tr;
	void *value = NULL;

	value = malloc(vtr->size);
	if (!value) {
		goto fail;
	}

	if (ax_trait_copy_or_init(vtr, value, val))
		goto fail;

	if (node->val) {
		vtr->free(node->val);
		free(node->val);
	} else {
		self->size ++;
	}
	node->val = value;

	return false;
fail:
	free(value);
	return true;
}

static struct node_st *make_path(ax_trie *trie, const ax_seq *key)
{
	ax_assert(trie->env.key_tr == key->env.box.elem_tr, "invalid element trait for the key");

	ax_btrie_r self_r = { .trie = trie };

	ax_citer key_it;
	struct node_st *last_node;
	int match_len = match_key(self_r.btrie, key, &key_it, &last_node);

	struct node_st *new_node_tab = NULL;
	void *value = NULL;
	
	size_t ins_count = ax_box_size(ax_cr(seq, key).box) + 1 - match_len;

	new_node_tab = malloc(sizeof(struct node_st) * ins_count);
	if (!new_node_tab) {
		goto fail;
	}
	memset(new_node_tab, 0, sizeof(struct node_st) * ins_count);

	int count = 0;
	for (size_t i = 0; i < ins_count; i++) {
		ax_map *new_submap = __ax_avl_construct(self_r.btrie->trie.env.key_tr, &node_tr);
		new_submap->env.box.any.one.scope.macro = self_r.one;
		if (!new_submap) {
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

	free(new_node_tab);
	return last_node;
fail:
	if (new_node_tab) {
		for (size_t i = 0; i < ins_count; i++)
			ax_one_free(new_node_tab[i].submap_r.one);
		free(new_node_tab);
	}
	free(value);
	return NULL;
}

static void *trie_put(ax_trie *trie, const ax_seq *key, const void *val)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);
	ax_assert(trie->env.key_tr == key->env.box.elem_tr, "invalid element trait for the key");

	ax_btrie_r self_r = { .trie = trie };

	struct node_st *node = make_path(trie, key);
	if (!node)
		return NULL;
	
	if (node_set_value(self_r.btrie, node, val))
		return NULL;

	return node->val;
}

static void *trie_get(const ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	return ax_iter_equal(&it, &end)
		? NULL
		: citer_get(ax_iter_cc(&it));
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

static bool trie_exist(const ax_trie *trie, const ax_seq *key, bool *valued)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	if (ax_iter_equal(&it, &end))
		return false;
	struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_c(&it));
	if (valued)
		*valued = !!node->val;
	return true;
}

static bool clean_path(ax_map *last) {
	ax_map_r parent_r = { .map = last };
	bool retval = false;

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
		struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_c(&it));
		if (node->val)
			return retval;
		
		ax_one_free(node->submap_r.one);
		ax_box_clear(parent_r.box);
		parent_r.one = (ax_one *)parent_r.one->env.scope.micro;
		retval = true;
	}
	return retval;
}

static bool trie_erase(ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	if (ax_iter_equal(&it, &end))
		return false;

	ax_btrie * self = (ax_btrie *)trie;
	struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_c(&it));
	node_free_value(self, node);
	if (ax_box_size(node->submap_r.box))
		return false;

	ax_one_free(node->submap_r.one);
	ax_avl_tr.box.iter.erase(&it);
	clean_path(node_get_parent(node));
	return true;
}

static void rec_remove(ax_btrie *self, ax_map *map)
{
	ax_map_r map_r = ax_r(map, map);
	ax_iter cur = ax_box_begin(map_r.box);
	ax_iter end = ax_box_end(map_r.box);
	while (!ax_iter_equal(&cur, &end)) {
		struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_c(&cur));
		rec_remove(self, node->submap_r.map);
		node_free_value(self, node);
		ax_one_free(node->submap_r.one);
		ax_avl_tr.box.iter.erase(&cur);
	}
}

static bool trie_prune(ax_trie *trie, const ax_seq *key)
{
	CHECK_PARAM_NULL(trie);
	CHECK_PARAM_NULL(key);

	ax_btrie *self = (ax_btrie *) trie;
	ax_iter it = trie_at(trie, key);
	ax_iter end = box_end((ax_box *)trie);
	if (ax_iter_equal(&it, &end))
		return false;
	struct node_st *node = ax_avl_tr.box.iter.get(ax_iter_c(&it));
	rec_remove(self, node->submap_r.map);
	return true;
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
		return false;

	struct node_st *old_node = ax_avl_tr.box.iter.get(ax_iter_c(&it));
	if (!old_node->val)
		return false;

	void *value = old_node->val;

	struct node_st *new_node = make_path(trie, key_to);
	if (!new_node)
		return true;

	new_node->val = value;
	old_node->val = NULL;
	iter_erase(&it);

	return true;
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

	struct node_st *node = ax_avl_tr.box.iter.get(it);
	ax_iter ret = ax_box_begin(node->submap_r.box);
	ret.tr = &ax_btrie_tr.box.iter;
	return ret;
}

static ax_iter trie_it_end(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));

	struct node_st *node = ax_avl_tr.box.iter.get(it);
	ax_iter ret = ax_box_end(node->submap_r.box);
	ret.tr = &ax_btrie_tr.box.iter;
	return ret;
}

static bool  trie_it_parent(const ax_citer *it, ax_iter *parent)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));
	UNSUPPORTED();

	return false;
}

static bool trie_it_valued(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, ax_one_is(it->owner, AX_AVL_NAME));

	struct node_st *node = ax_avl_tr.box.iter.get(it);
	return !!node->val;
}


static const ax_trait node_tr = { 
	.size  = sizeof(struct node_st),
	.equal = NULL,
	.less  = NULL,
	.hash  = NULL,
	.free  = ax_trait_mem_free,
	.copy  = ax_trait_mem_copy,
	.init  = ax_trait_mem_init,
	.link  = false
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
		},
		.iter = {
			.norm = true,
			.type = AX_IT_RAND,
			.next = citer_next,
			.prev = citer_prev,
			.box = citer_box,
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

ax_trie *__ax_btrie_construct(const ax_trait *key_tr, const ax_trait *val_tr)
{
	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(key_tr->equal);
	CHECK_PARAM_NULL(key_tr->hash);
	CHECK_PARAM_NULL(key_tr->copy);
	CHECK_PARAM_NULL(key_tr->free);

	CHECK_PARAM_NULL(val_tr);
	CHECK_PARAM_NULL(val_tr->copy);
	CHECK_PARAM_NULL(val_tr->free);

	ax_map *root = NULL;
	ax_btrie *self = NULL;

	self = malloc(sizeof(ax_btrie));
	if (!self) {
		goto fail;
	}

	root = ax_class_new(avl, ax_t(nil), &node_tr).map;
	if (!root) {
		goto fail;
	}
	root->env.box.any.one.scope.macro = ax_r(btrie, self).one;

	ax_btrie btrie_init = {
		.trie = {
			.tr = &ax_btrie_tr,
			.env = {
				.key_tr = key_tr,
				.box.elem_tr = val_tr,
			},
		},
		.root_r.map = root,
		.size = 0,
		.capacity = SIZE_MAX,
	};
	memcpy(self, &btrie_init, sizeof btrie_init);
	return ax_r(btrie, self).trie;
fail:
	ax_one_free(ax_r(map, root).one);
	free(self);
	return NULL;
}


