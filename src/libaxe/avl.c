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

#include "check.h"

#include <axe/avl.h>
#include <axe/map.h>
#include <axe/iter.h>
#include <axe/scope.h>
#include <axe/pool.h>
#include <axe/debug.h>
#include <axe/base.h>
#include <axe/mem.h>
#include <axe/error.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

struct node_st
{
        struct node_st *left;
        struct node_st *right;
        struct node_st *parent;
        size_t height;
	ax_byte kvbuffer[];
};

struct ax_avl_st
{
	ax_map __map;
	ax_one_env one_env;
	struct node_st *root;
	size_t size;
};

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

static ax_fail  map_put (ax_map* map, const void *key, const void *val);
static ax_fail  map_erase (ax_map* map, const void *key);
static void    *map_get (const ax_map* map, const void *key);
static ax_bool  map_exist (const ax_map* map, const void *key);

static size_t   box_size(const ax_box* box);
static size_t   box_maxsize(const ax_box* box);
static ax_iter  box_begin(const ax_box* box);
static ax_iter  box_end(const ax_box* box);
static ax_iter  box_rbegin(const ax_box* box);
static ax_iter  box_rend(const ax_box* box);
static void     box_clear(ax_box* box);
static const ax_stuff_trait *box_elem_tr(const ax_box *box);

static void     any_dump(const ax_any* any, int ind);
static ax_any  *any_copy(const ax_any* any);
static ax_any  *any_move(ax_any* any);

static void     one_free(ax_one* one);

static void     iter_move(ax_iter *it, long i);
static void     iter_prev(ax_iter *it);
static void     iter_next(ax_iter *it);
static void    *iter_get(const ax_iter *it);
static ax_fail  iter_set(const ax_iter *it, const void *val);
static ax_bool  iter_equal(const ax_iter *it1, const ax_iter *it2);
static ax_bool  iter_less(const ax_iter *it1, const ax_iter *it2);
static long     iter_dist(const ax_iter *it1, const ax_iter *it2);
static void     iter_erase(ax_iter *it);

static void     riter_move(ax_iter *it, long i);
static void     riter_prev(ax_iter *it);
static void     riter_next(ax_iter *it);
static ax_bool  riter_less(const ax_iter *it1, const ax_iter *it2);
static long     riter_dist(const ax_iter *it1, const ax_iter *it2);

static const ax_iter_trait iter_trait;
static const ax_iter_trait riter_trait;
static const ax_one_trait one_trait;
static const ax_any_trait any_trait;
static const ax_box_trait box_trait;
static const ax_map_trait map_trait;

inline static void *get_node_value(const ax_map *map, struct node_st *node)
{
	assert(node);
	return node->kvbuffer + map->key_tr->size;
}


inline static struct node_st *get_left_end_node(const ax_map *map, struct node_st *node)
{
	assert(node);
	while (node->left)
		node = node->left;
	return node;
}

inline static struct node_st *get_right_end_node(const ax_map *map, struct node_st *node)
{
	assert(node);
	while (node->right)
		node = node->right;
	return node;
}

static struct node_st *get_left_node(const ax_map *map, struct node_st *node)
{
	assert(node);
	if (node->left) {
		node = node->left;
		while (node->right)
			node = node->right;
	} else {
		if (!node->parent)
			return NULL;
		while (node->parent && node->parent->left == node)
			node = node->parent;
		if (node->parent && node->parent->right == node)
			node = node->parent;
		else
			return NULL;
	}
	return node;
}

static struct node_st *get_right_node(const ax_map *map, struct node_st *node)
{
	assert(node);
	if (node->right) {
		node = node->right;
		while (node->left)
			node = node->left;
	} else {
		if (!node->parent)
			return NULL;
		while (node->parent && node->parent->right == node)
			node = node->parent;
		if (node->parent && node->parent->left == node)
			node = node->parent;
		else
			return NULL;
	}
	return node;
}

static struct node_st *find_node(const ax_map *map, struct node_st *node, const void *key)
{

	if (node == NULL) return NULL;
	if (map->key_tr->less(key, node->kvbuffer, map->key_tr->size))
		return find_node(map, node->left, key);
	else if (map->key_tr->less(node->kvbuffer, key, map->key_tr->size))
		return find_node(map, node->right, key);
	else
		return node;
}

inline static int height(struct node_st *root)
{
	return root ? root->height : 0;
}

inline static void adjust_height(struct node_st *root)
{
	root->height = 1 + AX_MAX(height(root->left), height(root->right));
}

static struct node_st *rotate_right(struct node_st *root)
{
	struct node_st *new_root = root->left;
	if (root->parent)
	{
		if (root->parent->left == root)
			root->parent->left = new_root;
		else
			root->parent->right = new_root;
	}
	new_root->parent = root->parent;
	root->parent = new_root;
	root->left = new_root->right;
	if (root->left)
		root->left->parent = root;
	new_root->right = root;

	adjust_height(root);
	adjust_height(new_root);
	return new_root;
}

static struct node_st *rotate_left(struct node_st *root)
{
	struct node_st *new_root = root->right;
	if (root->parent)
	{
		if (root->parent->right == root)
			root->parent->right = new_root;
		else
			root->parent->left = new_root;
	}
	new_root->parent = root->parent;
	root->parent = new_root;
	root->right = new_root->left;
	if (root->right)
		root->right->parent = root;
	new_root->left = root;

	adjust_height(root);
	adjust_height(new_root);
	return new_root;
}

static struct node_st *make_node(ax_map *map, struct node_st *parent, const void *key, const void *value)
{
	ax_base *base = ax_one_base(ax_cast(map, map).one);
	ax_pool *pool = ax_base_pool(base);

	struct node_st *node = ax_pool_alloc(pool, sizeof(struct node_st) + map->key_tr->size + map->val_tr->size);
	if (node == NULL)
		goto failed;

	node->parent = parent;
	node->height = 1;
	node->left = NULL;
	node->right = NULL;
	if(map->key_tr->copy(pool, node->kvbuffer, key, map->key_tr->size))
		goto failed;
	if(map->val_tr->copy(pool, get_node_value(map, node), value, map->val_tr->size))
		goto failed;
	return node;
failed:
	if (node)
		ax_pool_free(node);
	ax_base_set_errno(base, AX_ERR_NOMEM);
	return NULL;
}

static struct node_st *balance(struct node_st *root)
{
	if (height(root->left) - height(root->right) > 1) {
		if (height(root->left->left) > height(root->left->right)) {
			root = rotate_right(root);
		} else {
			rotate_left(root->left);
			root = rotate_right(root);
		}
	}
	else if (height(root->right) - height(root->left) > 1) {
		if (height(root->right->right) > height(root->right->left)) {
			root = rotate_left(root);
		} else {
			rotate_right(root->right);
			root = rotate_left(root);
		}
	}
	return root;
}

static void swap_node(ax_map *map, struct node_st *node1, struct node_st *node2)
{
	ax_avl_role avl_r = { .map = map };
	size_t node_size = sizeof(struct node_st) + avl_r.map->key_tr->size + avl_r.map->val_tr->size;

	ax_memxor(node1, node2, node_size);
	ax_memxor(node2, node1, node_size);
	ax_memxor(node1, node2, node_size);
}

static struct node_st* remove_node(ax_map *map, struct node_st* node)
{

	struct node_st * new_node = node, **pnode = NULL;
	if(node->parent) {
		if(node->parent->left == node)
			pnode = &node->parent->left;
		else
			pnode = &node->parent->right;
	}
	
	if(node->left == NULL && node->right == NULL) {
		if(pnode)
			*pnode = NULL;
		new_node = node->parent;
	} else if(node->left == NULL) {
		node->right->parent = node->parent;
		if(pnode)
			*pnode = node->right;
		new_node = node->right;
	} else if(node->right == NULL) {
		node->left->parent = node->parent;
		if(pnode)
			*pnode = node->left;
		new_node = node->left;
	} else {
		struct node_st * greater_node = node->right; 
		while(greater_node->left) greater_node = greater_node->left;

		swap_node(map, greater_node, node);
		if (node->parent == greater_node)
		{
			greater_node->right = node->right;
			if(node->right)
				node->right->parent = greater_node; 
			new_node = greater_node;
		}
		else
		{
			node->parent->left = node->right;
			if(node->right)
				node->right->parent = node->parent;
			new_node = node->parent;
		}
	}
	map->key_tr->free(node->kvbuffer);
	map->val_tr->free(get_node_value(map, node));
	ax_pool_free(node);
	return new_node;
}

static void remove_child(struct node_st *node) {
	if (node) {
		remove_child(node->left);
		remove_child(node->right);
		ax_pool_free(node);
	}
}

static void iter_move(ax_iter *it, long i)
{
	UNSUPPORTED();
}

static void iter_prev(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_avl_role avl_r = { it->owner };
	it->point = it->point ? get_left_node(avl_r.map, it->point)
		: get_right_end_node(avl_r.map, avl_r.avl->root);
}

static void iter_next(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_assert(it->point != NULL, "iterator boundary exceeded");
	ax_avl* avl= (ax_avl*)it->owner;
	it->point =  get_right_node(&avl->__map, it->point);
}

static void *iter_get(const ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->point && it->tr);

	const ax_avl *avl= it->owner;
	ax_base *base = ax_one_base(ax_ccast(avl, avl).one);

	struct node_st *node = it->point;

	void *pkey = node->kvbuffer;
	void *pval = get_node_value(it->owner, node);

	void *key = avl->__map.key_tr->link ? *(void**)pkey : pkey;
	void *val = avl->__map.val_tr->link ? *(void**)pval : pval;

	ax_pair_role pair_role = ax_pair_create(ax_base_local(base), key, val);
	return pair_role.pair;
}

static ax_fail iter_set(const ax_iter *it, const void *val)
{
	CHECK_PARAM_NULL(val);
	CHECK_PARAM_VALIDITY(it, it->owner && it->point && it->tr);

	ax_avl_role avl_r = { .one = (ax_one *)it->owner };
	ax_base *base = ax_one_base(avl_r.one);
	ax_pool *pool = ax_base_pool(base);
	const ax_stuff_trait *val_tr = avl_r.map->val_tr;
	const void *psrc = val_tr->link ? &val : val;
	void *pdst = get_node_value(avl_r.map, it->point);
	val_tr->free(pdst);
	if (val_tr->copy(pool, pdst, psrc,  val_tr->size)) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}
	return ax_false;
}

 
static ax_bool iter_equal(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	return it1->point == it2->point;
}

static ax_bool iter_less(const ax_iter *it1, const ax_iter *it2)
{
	UNSUPPORTED();
	return ax_false;
}

static long iter_dist(const ax_iter *it1, const ax_iter *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	ax_avl_role avl_r = { .one = (ax_one *)it1->owner};

	struct node_st *node1 = it1->point;
	struct node_st *node2 = it2->point;
	struct node_st *cur = get_left_end_node(avl_r.map, avl_r.avl->root);

	size_t loc1, loc2;
	loc1 = loc2 = avl_r.avl->size;

	int found = !node1 + !node2;

	for (int i = 0; found < 2 && cur; i++) {
		if (cur == node1)
			loc1 = i, found++;
		if (cur == node2)
			loc2 = i, found++;
		cur = get_right_node(avl_r.map, cur);
	}

	ax_assert(found == 2, "bad iterator");

	return loc2 - loc1;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_NULL(it->point);

	ax_avl_role avl_r = { it->owner };
	struct node_st * node = it->point;

	struct node_st *next_node = (it->tr->norm)
		? get_right_node(avl_r.map, node)
		: get_left_node(avl_r.map, node);

	it->point = next_node;

	struct node_st * current = remove_node(avl_r.map, node);

	if (current) {
		while (current->parent) {
			adjust_height(current);
			current = balance(current);
			current  = current->parent;
		} 
		adjust_height(current);
		current = balance(current);
	}
	avl_r.avl->root = current;
	avl_r.avl->size --;
}

static void riter_move(ax_iter *it, long i)
{
	UNSUPPORTED();
}

static void riter_prev(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_avl* avl= it->owner;
	it->point = it->point ? get_right_node(&avl->__map, it->point)
		: get_left_end_node(&avl->__map, avl->root);
}

static void riter_next(ax_iter *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_assert(it->point != NULL, "iterator boundary exceeded");
	ax_avl *avl = it->owner;
	it->point =  get_left_node(&avl->__map, it->point);
}

static ax_bool riter_less(const ax_iter *it1, const ax_iter *it2)
{
	UNSUPPORTED();
	return ax_false;
}

static long riter_dist(const ax_iter *it1, const ax_iter *it2)
{
	return iter_dist(it2, it1);
}

static ax_fail map_put (ax_map* map, const void *key, const void *val)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);
	CHECK_PARAM_NULL(val);

	ax_avl_role avl_r = { .map = map };
	ax_base *base = ax_one_base(avl_r.one);
	ax_pool* pool = ax_base_pool(base);

	const void *pkey = map->key_tr->link ? &key : key;
	const void *pval = map->val_tr->link ? &val : val;

	struct node_st *current = avl_r.avl->root;


	if(avl_r.avl->size == 0) {
		avl_r.avl->root = make_node(map, NULL, pkey, pval);
		if (avl_r.avl->root == NULL) {
			ax_base_set_errno(base, AX_ERR_NOMEM);
			return ax_false;
		}
		avl_r.avl->size = 1;
		return ax_false;
	}

	ax_bool greater, lesser;
	while (ax_true) {
		lesser = map->key_tr->less(pkey, current->kvbuffer, map->key_tr->size);
		greater = map->key_tr->less(current->kvbuffer, pkey, map->key_tr->size);
		if (!lesser && !greater) {
			break;
		} else if (lesser) {
			if (current->left)
				current = current->left;
			else {
				current->left = make_node(map, current, pkey, pval);
				if (current->left == NULL) {
					ax_base_set_errno(base, AX_ERR_NOMEM);
					return ax_true;
				}
				current = current->left;
				break;
			}
		} else if (greater) {
			if (current->right) 
				current = current->right;
			else {
				current->right = make_node(map, current, pkey, pval);
				if (current->right == NULL) {
					ax_base_set_errno(base, AX_ERR_NOMEM);
					return ax_true;
				}
				current = current->right;
				break;
			}
		} else {
			map->val_tr->copy(pool, get_node_value(map, current), pval, map->val_tr->size);
			return ax_false;
		}
	}
	do {
		current  = current->parent;
		adjust_height(current);
		current = balance(current);
	} while (current->parent);

	avl_r.avl->root = current;
	avl_r.avl->size ++;

	return ax_false;
}

static ax_fail map_erase (ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);

	ax_avl_role avl_r = { .map = map };
	const void *pkey = map->key_tr->link ? &key : key;

	struct node_st * node = find_node(map, avl_r.avl->root, pkey);
	if (!node) {
		ax_base *base = ax_one_base(avl_r.one);
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}

	struct node_st * current = remove_node(map, node);

	if (current) {
		while (current->parent) {
			adjust_height(current);
			current = balance(current);
			current  = current->parent;
		} 
		adjust_height(current);
		current = balance(current);
	}

	avl_r.avl->root = current;
	avl_r.avl->size --;

	return ax_false;
}

static void *map_get (const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);

	ax_avl_role avl_r = { .map = (ax_map*)map };
	const void *pkey = map->key_tr->link ? &key : key;

	struct node_st *node = find_node(map, avl_r.avl->root, pkey);
	if(!node)
		return NULL;

	void *pval = get_node_value(map, node);
	void *val = map->key_tr->link ? *(void **)pval : pval;

	return val;
}

static ax_bool map_exist (const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);

	const void *pkey = map->key_tr->link ? &key : key;
	ax_avl_role avl_r = { .map = (ax_map *)map };
	return !!find_node(map, avl_r.avl->root, pkey);
}


static void one_free(ax_one* one)
{
	if (!one)
		return;
	ax_avl_role avl_r = { .one = one };
	ax_scope_detach(one);
	box_clear(avl_r.box);
	ax_pool_free(one);

}

static void any_dump(const ax_any* any, int ind)
{
	fprintf(stderr, "not implemented\n");
}


static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_avl_role src_r = { .any = (ax_any *)any };
	ax_base *base = ax_one_base(src_r.one);
	const ax_stuff_trait *ktr = src_r.map->key_tr;
	const ax_stuff_trait *vtr = src_r.map->val_tr;
	ax_avl_role dst_r = { .map = __ax_avl_construct(base, ktr, vtr)};

	ax_foreach(ax_pair *, pair, src_r.box) {
		if (ax_map_put(dst_r.map, ax_pair_key(pair), ax_pair_value(pair))) {
			ax_one_free(dst_r.one);
			return NULL;
		}
	}

	return dst_r.any;
}

static ax_any *any_move(ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_avl_role src_r = { .any = any };
	ax_base *base = ax_one_base(src_r.one);
	ax_pool *pool = ax_base_pool(base);

	ax_avl *dst = ax_pool_alloc(pool, sizeof(ax_avl));
	memcpy(dst, src_r.avl, sizeof(ax_avl));
	src_r.avl->size = 0;


	dst->one_env.scope = NULL;
	dst->one_env.sindex = 0;

	ax_scope_attach(ax_base_local(base), ax_cast(avl, dst).one);

	return (ax_any *) dst;
}


static size_t box_size(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_role avl_r = { .box = (ax_box*)box };
	return avl_r.avl->size;
}

static size_t box_maxsize(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	return 0xFFFF;
}

static ax_iter box_begin(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_role avl_r = { .box = (ax_box*)box };

	struct node_st *node = avl_r.avl->root;
	if (node)
		while (node->left)
			node = node->left;

	ax_iter it = {
		.owner = (void *)box,
		.point = node,
		.tr = &iter_trait
	};

	return it;
}

static ax_iter box_end(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = (void *)box,
		.point = NULL,
		.tr = &iter_trait,
	};
	return it;
}

static ax_iter box_rbegin(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_role avl_r = { .box = (ax_box*)box };

	struct node_st *node = avl_r.avl->root;
	if (node)
		while (node->right)
			node = node->right;

	ax_iter it = {
		.owner = (void *)box,
		.point = node,
		.tr = &riter_trait
	};
	return it;
}

static ax_iter box_rend(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	
	ax_iter it = {
		.owner = (void *)box,
		.point = NULL,
		.tr = &riter_trait,
	};
	return it;
}

static void box_clear(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_role avl_r = { .box = (ax_box*)box };

	ax_iter cur = ax_box_begin(avl_r.box);
	ax_iter last = ax_box_end(avl_r.box);
	while (!ax_iter_equal(cur, last)) {
		struct node_st *node = cur.point;
		avl_r.map->key_tr->free(node->kvbuffer);
		avl_r.map->val_tr->free(node->kvbuffer + avl_r.map->key_tr->size);
		cur = ax_iter_next(cur);
	}

	remove_child(avl_r.avl->root);
	avl_r.avl->size = 0;
}

static const ax_stuff_trait *box_elem_tr(const ax_box *box)
{
	 ax_avl_role avl_r = { .box = (ax_box*)box };
	return avl_r.map->val_tr;
}

static const ax_one_trait one_trait =
{
	.name  = "one.any.box.map.avl",
	.free  = one_free,
	.envp  = offsetof(ax_avl, one_env)
};

static const ax_any_trait any_trait =
{
	.dump = any_dump,
	.copy = any_copy,
	.move = any_move,
};

static const ax_box_trait box_trait =
{
	.size    = box_size,
	.maxsize = box_maxsize,
	.begin   = box_begin,
	.end     = box_end,
	.rbegin  = box_rbegin,
	.rend    = box_rend,
	.clear   = box_clear,
	.elem_tr = box_elem_tr
};

static const ax_map_trait map_trait =
{
	.put   = map_put,
	.get   = map_get,
	.erase = map_erase,
	.exist = map_exist,
};

static const ax_iter_trait iter_trait =
{
	.norm   = ax_true,
	.type   = AX_IT_BID,

	.move   = iter_move,
	.prev = iter_prev,
	.next = iter_next,

	.get    = iter_get,
	.set    = iter_set,
	.erase  = iter_erase,

	.equal  = iter_equal,
	.less   = iter_less,
	.dist   = iter_dist
};

static const ax_iter_trait riter_trait =
{
	.norm   = ax_false,
	.type   = AX_IT_BID,

	.move   = riter_move,
	.prev = riter_prev,
	.next = riter_next,

	.get    = iter_get,
	.set    = iter_set,
	.erase  = iter_erase,

	.equal  = iter_equal,
	.less   = riter_less,
	.dist   = riter_dist
};

ax_map *__ax_avl_construct(ax_base* base, const ax_stuff_trait* key_tr, const ax_stuff_trait* val_tr)
{
	CHECK_PARAM_NULL(base);

	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(val_tr->less);
	CHECK_PARAM_NULL(val_tr->copy);
	CHECK_PARAM_NULL(val_tr->free);

	CHECK_PARAM_NULL(val_tr);
	CHECK_PARAM_NULL(val_tr->copy);
	CHECK_PARAM_NULL(val_tr->free);


	ax_avl *avl = ax_pool_alloc(ax_base_pool(base), sizeof(ax_avl));
	if (avl == NULL)
		return NULL;
	
	ax_avl avl_init = {
		.__map = {
			.__box = {
				.__any = {
					.__one = {
						.base = base,
						.tr = &one_trait
					},
					.tr = &any_trait,
				},
				.tr = &box_trait,
			},
			.tr = &map_trait,
			.key_tr = key_tr,
			.val_tr = val_tr,
		},
		.one_env = {
			.scope = NULL,
			.sindex = 0
		},
		.size = 0,
		.root = NULL
	};

	memcpy(avl, &avl_init, sizeof avl_init);
	return &avl->__map;
}

ax_avl_role ax_avl_create(ax_scope *scope, const ax_stuff_trait *key_tr, const ax_stuff_trait *val_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(val_tr);

	ax_base *base = ax_one_base(ax_cast(scope, scope).one);
	ax_avl_role avl_r =  { .map = __ax_avl_construct(base, key_tr, val_tr) };
	if (avl_r.one == NULL)
		return avl_r;
	ax_scope_attach(scope, avl_r.one);
	return avl_r;
}
