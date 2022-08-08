/*
 * Copyright (c) 2020 - 2021 Li hsilin <lihsilyn@gmail.com>
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

#include "ax/avl.h"
#include "ax/iter.h"
#include "ax/debug.h"
#include "ax/mem.h"
#include "ax/trait.h"
#include "check.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#undef free

ax_concrete_begin(ax_avl)
	struct node_st {
		struct node_st *left;
		struct node_st *right;
		struct node_st *parent;
		size_t height;
		ax_byte kvbuffer[];
	} *root;
	size_t size;
ax_end;	

static void *map_put(ax_map* map, const void *key, const void *val, va_list *ap);
static ax_fail map_erase(ax_map* map, const void *key);
static void *map_get(const ax_map* map, const void *key);
static ax_iter map_at(const ax_map* map, const void *key);
static bool map_exist(const ax_map* map, const void *key);
static const void *map_it_key(const ax_citer *it);
static size_t box_size(const ax_box* box);
static size_t box_maxsize(const ax_box* box);
static ax_iter box_begin(ax_box* box);
static ax_iter box_end(ax_box* box);
static ax_iter box_rbegin(ax_box* box);
static ax_iter box_rend(ax_box* box);
static void box_clear(ax_box* box);
static ax_dump *any_dump(const ax_any* any);
static ax_any *any_copy(const ax_any* any);
static void one_free(ax_one* one);
static const char *one_name(const ax_one *one);
static void citer_prev(ax_citer *it);
static void citer_next(ax_citer *it);
static bool citer_less(const ax_citer *it1, const ax_citer *it2);
static long citer_dist(const ax_citer *it1, const ax_citer *it2);
static void rciter_prev(ax_citer *it);
static void rciter_next(ax_citer *it);
static bool rciter_less(const ax_citer *it1, const ax_citer *it2);
static long rciter_dist(const ax_citer *it1, const ax_citer *it2);
static void *citer_get(const ax_citer *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);
static void iter_erase(ax_iter *it);
inline static void *node_val(const ax_map *map, struct node_st *node);
static void *node_change_value(ax_map *map, struct node_st *node, const void *val, va_list *ap);

inline static void *node_val(const ax_map *map, struct node_st *node)
{
	assert(node);
	return node->kvbuffer + ax_class_env(map).key_tr->size;
}

inline static void *node_key(struct node_st *node)
{
	assert(node);
	return node->kvbuffer;
}

inline static struct node_st *lowest_node(const ax_map *map, struct node_st *node)
{
	assert(node);
	while (node->left)
		node = node->left;
	return node;
}

inline static struct node_st *highest_node(const ax_map *map, struct node_st *node)
{
	assert(node);
	while (node->right)
		node = node->right;
	return node;
}

static struct node_st *lower_node(const ax_map *map, struct node_st *node)
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

static struct node_st *higher_node(const ax_map *map, struct node_st *node)
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
	const ax_trait *ktr = ax_class_env(map).key_tr;
	if (ax_trait_less(ktr, key, node->kvbuffer))
		return find_node(map, node->left, key);
	else if (ax_trait_less(ktr, node->kvbuffer, key))
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
	if (root->parent) {
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
	if (root->parent) {
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

static struct node_st *make_node(ax_map *map, struct node_st *parent, const void *key, const void *value, va_list *ap)
{
	ax_avl_r self = { map };
	const ax_trait *etr = ax_class_env(self.ax_box).elem_tr;
	struct node_st *node = malloc(sizeof(struct node_st) + ax_class_env(map).key_tr->size + etr->size);
	if (node == NULL)
		goto failed;

	node->parent = parent;
	node->height = 1;
	node->left = NULL;
	node->right = NULL;
	const ax_trait *ktr = ax_class_env(map).key_tr;
	if(ax_trait_copy(ktr, node->kvbuffer, key))
		goto failed;
	if (ax_trait_copy_or_init(etr, node_val(map, node), value, ap))
		goto failed;
	return node;
failed:
	if (node)
		free(node);
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

static struct node_st* remove_node(ax_avl *avl, struct node_st* node)
{
	ax_avl_r self = ax_r(ax_avl, avl);

	struct node_st * dirty_node, **pnode = NULL;
	if(node->parent) {
		if(node->parent->left == node)
			pnode = &node->parent->left;
		else
			pnode = &node->parent->right;
	} else
		pnode = &avl->root;
	
	if(node->left == NULL && node->right == NULL) {
		*pnode = NULL;
		dirty_node = node->parent;
	} else if(node->left == NULL) {
		node->right->parent = node->parent;
		*pnode = node->right;
		dirty_node = node->parent ? node->parent : node->right;
	} else if(node->right == NULL) {
		node->left->parent = node->parent;
		*pnode = node->left;
		dirty_node = node->parent ? node->parent : node->left;
	} else {
		struct node_st * rnode = higher_node(self.ax_map, node);
		if (rnode->parent == node) {
			dirty_node = rnode;
			rnode->parent = node->parent;
			node->left->parent = rnode;
			rnode->left = node->left;
			
		} else {
			dirty_node = rnode->parent;
			rnode->parent->left = rnode->right;
			if (rnode->right)
				rnode->right->parent = rnode->parent;
			rnode->right = node->right;
			rnode->parent = node->parent;
			rnode->left = node->left;
			if (node->left)
				node->left->parent = rnode;
			node->right->parent = rnode;
		}
		*pnode = rnode;
	}

	ax_trait_free(ax_class_env(self.ax_map).key_tr, node->kvbuffer);
	ax_trait_free(ax_class_env(self.ax_box).elem_tr, node_val(self.ax_map, node));
	free(node);
	return dirty_node;
}

static void remove_child(struct node_st *node) {
	if (node) {
		remove_child(node->left);
		remove_child(node->right);
		free(node);
	}
}

static void citer_prev(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_avl_cr self = { it->owner };
	it->point = it->point ? lower_node(self.ax_map, it->point)
		: highest_node(self.ax_map, self.ax_avl->root);
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_assert(it->point != NULL, "iterator boundary exceeded");
	ax_avl* avl= (ax_avl*)it->owner;
	it->point =  higher_node(&avl->ax_map, it->point);
}

static bool citer_less(const ax_citer *it1, const ax_citer *it2)
{
	UNSUPPORTED();
	return false;
}

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	ax_avl_r self = AX_R_INIT(ax_one, (ax_one *)it1->owner);

	struct node_st *node1 = it1->point;
	struct node_st *node2 = it2->point;
	struct node_st *cur = lowest_node(self.ax_map, self.ax_avl->root);

	size_t loc1, loc2;
	loc1 = loc2 = self.ax_avl->size;

	int found = !node1 + !node2;

	for (int i = 0; found < 2 && cur; i++) {
		if (cur == node1)
			loc1 = i, found++;
		if (cur == node2)
			loc2 = i, found++;
		cur = higher_node(self.ax_map, cur);
	}

	ax_assert(found == 2, "bad iterator");

	return loc2 - loc1;
}

static void rciter_prev(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	const ax_avl* avl = it->owner;
	it->point = it->point ? higher_node(&avl->ax_map, it->point)
		: lowest_node(&avl->ax_map, avl->root);
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_assert(it->point != NULL, "iterator boundary exceeded");
	const ax_avl *avl = it->owner;
	it->point =  lower_node(&avl->ax_map, it->point);
}

static bool rciter_less(const ax_citer *it1, const ax_citer *it2)
{
	UNSUPPORTED();
	return false;
}

static long rciter_dist(const ax_citer *it1, const ax_citer *it2)
{
	return citer_dist(it2, it1);
}

static void *citer_get(const ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->point && it->tr);
	return node_val(it->owner, it->point);
}

static void *node_change_value(ax_map *map, struct node_st *node, const void *val, va_list *ap)
{

	ax_avl_r self = AX_R_INIT(ax_map, map);
	const ax_trait *vtr = ax_class_env(self.ax_box).elem_tr;
	void *dst = node_val(map, node);
	ax_trait_free(vtr, dst);
	if (ax_trait_copy_or_init(vtr, dst, val, ap))
		return NULL;
	return node_val(map, node);
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->point && it->tr);

	ax_avl_r self = AX_R_INIT(ax_one, it->owner);
	return !node_change_value(self.ax_map, it->point, val, ap);
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_NULL(it->point);

	ax_avl_r self = { it->owner };
	struct node_st * node = it->point;

	struct node_st *next_node = ax_iter_norm(it)
		? higher_node(self.ax_map, node)
		: lower_node(self.ax_map, node);

	it->point = next_node;

	struct node_st * current = remove_node(self.ax_avl, node);

	if (current) {
		while (current->parent) {
			adjust_height(current);
			current = balance(current);
			current  = current->parent;
		} 
		adjust_height(current);
		current = balance(current);
	}
	self.ax_avl->root = current;
	self.ax_avl->size --;
}

static void *map_put (ax_map* map, const void *key, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(map);

	ax_avl_r self = AX_R_INIT(ax_map, map);
	const ax_trait
		*ktr = ax_class_env(self.ax_map).key_tr,
		*vtr = ax_class_env(self.ax_box).elem_tr;
	struct node_st *current = self.ax_avl->root, *new_node = NULL;

	if(!self.ax_avl->root) {
		new_node = make_node(map, NULL, key, val, ap);
		if (new_node == NULL) {
			return false;
		}
		self.ax_avl->root = new_node;
		self.ax_avl->size = 1;
		return node_val(map, new_node);
	}

	bool greater, lesser;
	while (true) {
		lesser = ax_trait_less(ktr, key, node_key(current));
		greater = ax_trait_less(ktr, node_key(current), key);
		if (lesser) {
			if (current->left)
				current = current->left;
			else {
				new_node = make_node(map, current, key, val, ap);
				if (!new_node) {
					return NULL;
				}
				current->left = new_node;
				current = current->left;
				break;
			}
		} else if (greater) {
			if (current->right) 
				current = current->right;
			else {
				new_node = make_node(map, current, key, val, ap);
				if (!new_node) {
					return NULL;
				}
				current->right = new_node;
				current = current->right;
				break;
			}
		} else {
			if (ax_trait_copy_or_init(vtr, node_val(map, current), val, ap))
				return NULL;
			return node_val(map, current);
		}
	}

	if (!new_node)
		return node_change_value(map, current, val, ap);

	do {
		current  = current->parent;
		adjust_height(current);
		current = balance(current);
	} while (current->parent);
	self.ax_avl->size ++;
	self.ax_avl->root = current;
	return node_val(map, new_node);;
}

static ax_fail map_erase (ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_avl_r self = AX_R_INIT(ax_map, map);
	struct node_st * node = find_node(map, self.ax_avl->root, key);
	if (!node)
		return true;

	struct node_st * current = remove_node(self.ax_avl, node);

	if (current) {
		while (current->parent) {
			adjust_height(current);
			current = balance(current);
			current  = current->parent;
		} 
		adjust_height(current);
		current = balance(current);
	}

	self.ax_avl->root = current;
	self.ax_avl->size --;

	return false;
}

static void *map_get (const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_avl_cr avl = AX_R_INIT(ax_map, map);
	struct node_st *node = find_node(map, avl.ax_avl->root, key);
	if(!node)
		return NULL;
	return node_val(map, node);
}

static ax_iter  map_at(const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_avl_cr self = AX_R_INIT(ax_map, map);

	struct node_st *node = find_node(map, self.ax_avl->root, key);
	if(!node)
		return box_end((ax_box *)self.ax_box);
	return (ax_iter) {
		.owner = (void *)map,
		.point = node,
		.tr = &ax_avl_tr.ax_box.iter,
		.etr = ax_class_env(self.ax_box).elem_tr,
	};
}

static bool map_exist(const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);
	ax_avl_cr self = AX_R_INIT(ax_map, map);
	return !!find_node(map, self.ax_avl->root, key);
}

static const void *map_it_key(const ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->point && it->tr);
	CHECK_ITER_TYPE(it, one_name(NULL));
	const ax_map *map = it->owner;
	return ax_trait_out(ax_class_env(map).key_tr, node_key(it->point));
}

static void one_free(ax_one* one)
{
	if (!one)
		return;
	ax_avl_r self = AX_R_INIT(ax_one, one);
	box_clear(self.ax_box);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, ax_avl);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_map_cr self = AX_R_INIT(ax_any, any);
	return ax_map_dump(self.ax_map);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_avl_cr src = AX_R_INIT(ax_any, any);
	const ax_trait
		*ktr = ax_class_env(src.ax_map).key_tr,
		*vtr = ax_class_env(src.ax_box).elem_tr;
	ax_avl_r dst =  ax_new(ax_avl, ktr, vtr);
	if (ax_r_isnull(dst))
		return NULL;

	ax_map_cforeach(src.ax_map, const void *, key, const void *, val) {
		if (!map_put(dst.ax_map, key, val, NULL))
			goto fail;
	}

	ax_class_env(dst.ax_one).scope.macro = NULL;
	ax_class_env(dst.ax_one).scope.micro = 0;
	return dst.ax_any;
fail:
	ax_one_free(dst.ax_one);
	return NULL;
}

static size_t box_size(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_cr self = AX_R_INIT(ax_box, box);
	return self.ax_avl->size;
}

static size_t box_maxsize(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	return SIZE_MAX;
}

static ax_iter box_begin(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_r self = AX_R_INIT(ax_box, box);
	struct node_st *node = self.ax_avl->root;
	if (node)
		while (node->left)
			node = node->left;
	ax_iter it = {
		.owner = box,
		.point = node,
		.tr = &ax_avl_tr.ax_box.iter,
		.etr = ax_class_env(box).elem_tr,
	};
	return it;
}

static ax_iter box_end(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = box,
		.point = NULL,
		.tr = &ax_avl_tr.ax_box.iter,
		.etr = ax_class_env(box).elem_tr,
	};
	return it;
}

static ax_iter box_rbegin(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_r self = AX_R_INIT(ax_box, box);
	struct node_st *node = self.ax_avl->root;
	if (node)
		while (node->right)
			node = node->right;
	ax_iter it = {
		.owner = box,
		.point = node,
		.tr = &ax_avl_tr.ax_box.riter,
		.etr = ax_class_env(box).elem_tr,
	};
	return it;
}

static ax_iter box_rend(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = box,
		.tr = &ax_avl_tr.ax_box.riter,
		.point = NULL,
		.etr = ax_class_env(box).elem_tr,
	};
	return it;
}

static void box_clear(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_avl_r self = AX_R_INIT(ax_box, box);

	const ax_trait
		*ktr = ax_class_env(self.ax_map).key_tr,
		*vtr = ax_class_env(self.ax_box).elem_tr;

	ax_iter cur = ax_box_begin(self.ax_box);
	ax_iter last = ax_box_end(self.ax_box);
	while (!ax_iter_equal(&cur, &last)) {
		struct node_st *node = cur.point;
		ax_trait_free(ktr, node->kvbuffer);
		ax_trait_free(vtr, node->kvbuffer + ktr->size);
		ax_iter_next(&cur);
	}

	remove_child(self.ax_avl->root);
	self.ax_avl->root = NULL;
	self.ax_avl->size = 0;
}

const ax_map_trait ax_avl_tr =
{
	.ax_box = {
		.ax_any = {
			.ax_one = {
				.name  = one_name,
				.free  = one_free,
			},
			.dump = any_dump,
			.copy = any_copy,
		},
		.iter = {
			.norm = true,
			.type = AX_IT_BID,
			.move = NULL,
			.prev = citer_prev,
			.next = citer_next,
			.less = citer_less,
			.dist = citer_dist,
			.get    = citer_get,
			.set    = iter_set,
			.erase  = iter_erase,
		},
		.riter = {
			.norm = false,
			.type = AX_IT_BID,
			.move = NULL,
			.prev = rciter_prev,
			.next = rciter_next,
			.less = rciter_less,
			.dist = rciter_dist,
			.get    = citer_get,
			.set    = iter_set,
			.erase  = iter_erase,
		},

		.size    = box_size,
		.maxsize = box_maxsize,
		.begin   = box_begin,
		.end     = box_end,
		.rbegin  = box_rbegin,
		.rend    = box_rend,
		.clear   = box_clear,
	},
	.put   = map_put,
	.get   = map_get,
	.at    = map_at,
	.erase = map_erase,
	.exist = map_exist,
	.itkey = map_it_key,
};

ax_map *__ax_avl_construct(const ax_trait* key_tr, const ax_trait* val_tr)
{
	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(key_tr->less);
	CHECK_PARAM_NULL(key_tr->copy);
	CHECK_PARAM_NULL(key_tr->free);

	CHECK_PARAM_NULL(val_tr);
	CHECK_PARAM_NULL(val_tr->copy || val_tr->init);
	CHECK_PARAM_NULL(val_tr->free);

	ax_avl *avl = malloc(sizeof(ax_avl));
	if (avl == NULL)
		return NULL;
	
	ax_avl avl_init = {
		.ax_map = {
			.tr = &ax_avl_tr,
			.env.ax_box.elem_tr = val_tr,
			.env.key_tr = key_tr,
		},
		.size = 0,
		.root = NULL
	};

	memcpy(avl, &avl_init, sizeof avl_init);
	return &avl->ax_map;
}

