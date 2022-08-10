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

#include "ax/rb.h"
#include "ax/iter.h"
#include "ax/mem.h"
#include "ax/trait.h"
#include "ax/detect.h"
#include "check.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#if defined(AX_ARCH_AMD64) && defined(_LP64)
# define USE_AUGMENTED_PTR
# define COLOR_SHIFT 63
#endif

ax_concrete_begin(ax_rb)
	struct node_st {
		struct node_st *left, *right, *parent;
#ifndef USE_AUGMENTED_PTR
		unsigned int color;
#endif
		ax_byte kvbuffer[];
	} *root;
size_t size;
struct node_st *rightmost;
ax_end;	

#define COLOR_BLACK 0x0ull
#define COLOR_RED   0x1ull

#ifdef USE_AUGMENTED_PTR /* Should we augment the pointer with the color metadata */
# define PARENT_PTR_MASK                     ((1ull << COLOR_SHIFT) - 1)
# define _GET_PARENT_COLOR_MASK(__node)      (((uintptr_t)(__node)->parent) & ~PARENT_PTR_MASK)
# define NODE_GET_COLOR(_node)               ((((uintptr_t)(_node)->parent) >> COLOR_SHIFT) & 1)
# define NODE_SET_COLOR(_node, _color)       ax_block { (_node)->parent = (struct node_st *)((((uintptr_t)(_node)->parent) & PARENT_PTR_MASK) | ((_color) << COLOR_SHIFT)); }
# define NODE_GET_PARENT(_node)              ((struct node_st *)(((uintptr_t)(_node)->parent) & PARENT_PTR_MASK))
# define NODE_SET_PARENT(_node, _parent)     ax_block { (_node)->parent = (struct node_st *)((uintptr_t)(_parent) | _GET_PARENT_COLOR_MASK((_node))); }
#else
# define NODE_GET_COLOR(_node)               ((_node)->color)
# define NODE_SET_COLOR(_node, _color)       ax_block { ((_node)->color = (_color)); }
# define NODE_GET_PARENT(_node)              ((_node)->parent)
# define NODE_SET_PARENT(_node, _parent)     ax_block { ((_node)->parent = (_parent)); }
#endif

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
inline static struct node_st *lowest_node(struct node_st *node);
inline static struct node_st *highest_node(struct node_st *node);
inline static struct node_st *get_grandparent(struct node_st *node);
inline static void rotate_left(ax_rb *tree, struct node_st *node);
inline static void rotate_right(ax_rb *tree, struct node_st *node);
static struct node_st *rb_tree_find(const ax_rb *tree, const void *key);
static void insert_rebalance(ax_rb *tree, struct node_st *node);
static int rb_tree_find_or_insert(struct ax_rb_st  *tree, const void *key, struct node_st *new_candidate, struct node_st **value);
static struct node_st *rb_tree_find_successor(struct node_st *node);
static struct node_st *rb_tree_find_predecessor(struct node_st *node);



static struct node_st *rb_tree_find(const ax_rb *tree, const void *key)
{
	assert(tree != NULL);
	assert(key != NULL);

	ax_rb_cr self = AX_R_INIT(ax_rb, tree);

	struct node_st *node = tree->root;

	while (node != NULL) {
		if (ax_trait_equal(ax_class_env(self.ax_map).key_tr, key, node->kvbuffer)) {
			break;
		} else if (ax_trait_less(ax_class_env(self.ax_map).key_tr, key, node->kvbuffer)) {
			node = node->left;
		} else {
			/* Otherwise, we want the right node, and continue iteration */
			node = node->right;
		}
	}
	return node;
}

/* Helper function to get a node's grandparent */
inline static struct node_st *get_grandparent(struct node_st *node)
{
	struct node_st *parent_node = NODE_GET_PARENT(node);

	if (parent_node == NULL) {
		return NULL;
	}

	return NODE_GET_PARENT(parent_node);
}

/* Helper function to do a left rotation of a given node */
inline static void rotate_left(ax_rb *tree, struct node_st *node)
{
	struct node_st *x = node;
	struct node_st *y = x->right;

	x->right = y->left;

	if (y->left != NULL) {
		struct node_st *yleft = y->left;
		NODE_SET_PARENT(yleft, x);
	}

	NODE_SET_PARENT(y, NODE_GET_PARENT(x));

	if (NODE_GET_PARENT(x) == NULL) {
		tree->root = y;
	} else {
		struct node_st *xp = NODE_GET_PARENT(x);
		if (x == xp->left) {
			xp->left = y;
		} else {
			xp->right = y;
		}
	}

	y->left = x;
	NODE_SET_PARENT(x, y);
}

/* Helper function to do a right rotation of a given node */
inline static void rotate_right(ax_rb *tree, struct node_st *node)
{
	struct node_st *x = node;
	struct node_st *y = x->left;

	x->left = y->right;

	if (y->right != NULL) {
		struct node_st *yright = y->right;
		NODE_SET_PARENT(yright, x);
	}

	NODE_SET_PARENT(y, NODE_GET_PARENT(x));

	if (NODE_GET_PARENT(x) == NULL) {
		tree->root = y;
	} else {
		struct node_st *xp = NODE_GET_PARENT(x);
		if (x == xp->left) {
			xp->left = y;
		} else {
			xp->right = y;
		}
	}

	y->right = x;
	NODE_SET_PARENT(x, y);
}

/* Function to perform a RB tree rebalancing after an insertion */
static void insert_rebalance(ax_rb *tree, struct node_st *node)
{
	struct node_st *new_node_parent = NODE_GET_PARENT(node);

	if (new_node_parent != NULL && NODE_GET_COLOR(new_node_parent) != COLOR_BLACK) {
		struct node_st *pnode = node;

		/* Iterate until we're at the root (which we just color black) or
		 * until we the parent node is no longer red.
		 */
		while ((tree->root != pnode) && (NODE_GET_PARENT(pnode) != NULL) &&
				(NODE_GET_COLOR(
							NODE_GET_PARENT(pnode)) == COLOR_RED))
		{
			struct node_st *parent = NODE_GET_PARENT(pnode);
			struct node_st *grandparent = get_grandparent(pnode);
			struct node_st *uncle = NULL;
			int uncle_is_left;

			assert(NODE_GET_COLOR(pnode) == COLOR_RED);

			if (parent == grandparent->left) {
				uncle_is_left = 0;
				uncle = grandparent->right;
			} else {
				uncle_is_left = 1;
				uncle = grandparent->left;
			}

			/* Case 1: Uncle is not black */
			if (uncle && NODE_GET_COLOR(uncle) == COLOR_RED) {
				/* Color parent and uncle black */
				NODE_SET_COLOR(parent, COLOR_BLACK);
				NODE_SET_COLOR(uncle, COLOR_BLACK);

				/* Color Grandparent as Red */
				NODE_SET_COLOR(grandparent, COLOR_RED);
				pnode = grandparent;
				/* Continue iteration, processing grandparent */
			} else {
				/* Case 2 - node's parent is red, but uncle is black */
				if (!uncle_is_left && parent->right == pnode) {
					pnode = NODE_GET_PARENT(pnode);
					rotate_left(tree, pnode);
				} else if (uncle_is_left && parent->left == pnode) {
					pnode = NODE_GET_PARENT(pnode);
					rotate_right(tree, pnode);
				}

				/* Case 3 - Recolor and rotate*/
				parent = NODE_GET_PARENT(pnode);
				NODE_SET_COLOR(parent, COLOR_BLACK);

				grandparent = get_grandparent(pnode);
				NODE_SET_COLOR(grandparent, COLOR_RED);
				if (!uncle_is_left) {
					rotate_right(tree, grandparent);
				} else {
					rotate_left(tree, grandparent);
				}
			}
		}

		/* Make sure the tree root is black (Case 1: Continued) */
		struct node_st *tree_root = tree->root;
		NODE_SET_COLOR(tree_root, COLOR_BLACK);
	}
}

#if 0
/* Return 1 if exists duplicate node, otherwise return 0 */
static int rb_tree_insert(struct ax_rb_st *tree, const void *key, struct node_st *node)
{
	int rightmost = 1;
	struct node_st *nd = NULL;

	assert(tree != NULL);
	assert(node != NULL);

	node->left = NULL;
	node->right = NULL;
	node->parent = NULL;

	/* Case 1: Simplest case -- tree is empty */
	if (tree->root == NULL) {
		tree->root = node;
		tree->rightmost = node;
		NODE_SET_COLOR(node, COLOR_BLACK);
		return 0;
	}

	/* Otherwise, insert the node as you would typically in a BST */
	nd = tree->root;
	rightmost = 1;

	/* Insert a node into the tree as you normally would */
	while (nd != NULL) {
		if (ax_trait_equal(tree->map.env.key_tr, node->kvbuffer, nd->kvbuffer))
			return 1;

		if (ax_trait_less(tree->map.env.key_tr, node->kvbuffer, nd->kvbuffer)) {
			rightmost = 0;
			if (nd->left == NULL) {
				nd->left = node;
				break;
			} else {
				nd = nd->left;
			}
		} else {
			if (nd->right == NULL) {
				nd->right = node;
				break;
			} else {
				nd = nd->right;
			}
		}
	}

	NODE_SET_PARENT(node, nd);
	ax_trait_copy(tree->map.env.key_tr, node->kvbuffer, key);
	NODE_SET_COLOR(node, COLOR_RED);

	if (1 == rightmost) {
		tree->rightmost = node;
	}

	/* Rebalance the tree about the node we just added */
	insert_rebalance(tree, node);

	return 0;
}
#endif

/* Return 0 if node is found, otherwise return 1 */
static int rb_tree_find_or_insert(struct ax_rb_st  *tree, const void *key, struct node_st *new_candidate, struct node_st **value)
{
	assert(tree != NULL);
	assert(value != NULL);
	assert(new_candidate != NULL);

	ax_rb_r self = AX_R_INIT(ax_rb, tree);

	*value = NULL;

	struct node_st *node = tree->root;

	/* Case 1: Tree is empty, so we just insert the node */
	if (tree->root == NULL) {
		tree->root = new_candidate;
		//new_candidate->left = new_candidate->right = NULL;
		tree->rightmost = new_candidate;
		NODE_SET_COLOR(new_candidate, COLOR_BLACK);
		*value = new_candidate;
		return 1;
	}

	struct node_st *node_prev = NULL;
	int dir = 0, rightmost = 1;
	while (node != NULL) {

		if (ax_trait_equal(ax_class_env(self.ax_map).key_tr, key, node->kvbuffer)) {
			break;
		} else if (ax_trait_less(ax_class_env(self.ax_map).key_tr, key, node->kvbuffer)) {
			node_prev = node;
			dir = 0;
			node = node->left;
			rightmost = 0;
		} else {
			/* Otherwise, we want the right node, and continue iteration */
			node_prev = node;
			dir = 1;
			node = node->right;
		}
	}

	if (node) {
		*value = node;
		return 0;
	}

	/* Case 2 - we didn't find the node, so insert the candidate */
	if (dir == 0) {
		rightmost = 0;
		node_prev->left = new_candidate;
	} else {
		node_prev->right = new_candidate;
	}

	NODE_SET_PARENT(new_candidate, node_prev);
	//ax_trait_copy(tree->map.env.key_tr, new_candidate->kvbuffer, key);

	node = new_candidate;

	NODE_SET_COLOR(node, COLOR_RED);

	if (1 == rightmost) {
		tree->rightmost = new_candidate;
	}

	/* Rebalance the tree, preserving rb properties */
	insert_rebalance(tree, node);
	*value = node;
	return 1;
}

static struct node_st *rb_tree_find_successor(struct node_st *node)
{
	struct node_st *x = node;

	if (x->right != NULL) {
		return lowest_node(x->right);
	}

	struct node_st *y = NODE_GET_PARENT(x);

	while (y != NULL && x == y->right) {
		x = y;
		y = NODE_GET_PARENT(y);
	}

	return y;
}

static struct node_st *rb_tree_find_predecessor(struct node_st *node)
{
	struct node_st *x = node;

	if (x->left != NULL) 
		return highest_node(x->left);

	struct node_st *y = NODE_GET_PARENT(x);

	while (y != NULL && x == y->left)
		x = y, y = NODE_GET_PARENT(y);

	return y;
}

/* Replace x with y, inserting y where x previously was */
static void swap_node(struct ax_rb_st *tree, struct node_st *x, struct node_st *y)
{
	struct node_st *left = x->left;
	struct node_st *right = x->right;
	struct node_st *parent = NODE_GET_PARENT(x);

	NODE_SET_PARENT(y, parent);

	if (parent != NULL) {
		if (parent->left == x)
			parent->left = y;
		else
			parent->right = y;
	} else {
		if (tree->root == x)
			tree->root = y;
	}

	y->right = right;
	if (right != NULL)
		NODE_SET_PARENT(right, y);

	x->right = NULL;

	y->left = left;
	if (left != NULL)
		NODE_SET_PARENT(left, y);
	x->left = NULL;

	NODE_SET_COLOR(y, NODE_GET_COLOR(x));
	x->parent = NULL;
}

static void delete_rebalance(struct ax_rb_st *tree, struct node_st *node, struct node_st *parent, int node_is_left)
{
	struct node_st *x = node;
	struct node_st *xp = parent;
	int is_left = node_is_left;

	while (x != tree->root && (x == NULL || NODE_GET_COLOR(x) == COLOR_BLACK)) {
		struct node_st *w = is_left ? xp->right : xp->left;    /* Sibling */

		if (w != NULL && NODE_GET_COLOR(w) == COLOR_RED) {
			/* Case 1: */
			NODE_SET_COLOR(w, COLOR_BLACK);
			NODE_SET_COLOR(xp, COLOR_RED);
			if (is_left)
				rotate_left(tree, xp);
			else
				rotate_right(tree, xp);
			w = is_left ? xp->right : xp->left;
		}

		struct node_st *wleft = w != NULL ? w->left : NULL;
		struct node_st *wright = w != NULL ? w->right : NULL;
		if ( (wleft == NULL || NODE_GET_COLOR(wleft) == COLOR_BLACK) &&
				(wright == NULL || NODE_GET_COLOR(wright) == COLOR_BLACK) )
		{
			/* Case 2: */
			if (w != NULL)
				NODE_SET_COLOR(w, COLOR_RED);
			x = xp;
			xp = NODE_GET_PARENT(x);
			is_left = xp && (x == xp->left);
		} else {
			if (is_left && (wright == NULL || NODE_GET_COLOR(wright) == COLOR_BLACK)) {
				/* Case 3a: */
				NODE_SET_COLOR(w, COLOR_RED);
				if (wleft)
					NODE_SET_COLOR(wleft, COLOR_BLACK);
				rotate_right(tree, w);
				w = xp->right;
			} else if (!is_left && (wleft == NULL || NODE_GET_COLOR(wleft) == COLOR_BLACK)) {
				/* Case 3b: */
				NODE_SET_COLOR(w, COLOR_RED);
				if (wright)
					NODE_SET_COLOR(wright, COLOR_BLACK);
				rotate_left(tree, w);
				w = xp->left;
			}

			/* Case 4: */
			wleft = w->left;
			wright = w->right;

			NODE_SET_COLOR(w, NODE_GET_COLOR(xp));
			NODE_SET_COLOR(xp, COLOR_BLACK);

			if (is_left && wright != NULL) {
				NODE_SET_COLOR(wright, COLOR_BLACK);
				rotate_left(tree, xp);
			} else if (!is_left && wleft != NULL) {
				NODE_SET_COLOR(wleft, COLOR_BLACK);
				rotate_right(tree, xp);
			}
			x = tree->root;
		}
	}

	if (x != NULL) {
		NODE_SET_COLOR(x, COLOR_BLACK);
	}
}

static void rb_tree_remove(struct ax_rb_st *tree, struct node_st *node)
{
	assert(tree != NULL);
	assert(node != NULL);

	struct node_st *y;

	if (node->left == NULL || node->right == NULL) {
		y = node;
		if (node == tree->rightmost)
			/* The new rightmost item is our successor */
			tree->rightmost = rb_tree_find_predecessor(node);
	} else {
		y = rb_tree_find_successor(node);
	}

	struct node_st *x, *xp;

	x = (y->left != NULL) ?  y->left : y->right;

	if (x != NULL) {
		NODE_SET_PARENT(x, NODE_GET_PARENT(y));
		xp = NODE_GET_PARENT(x);
	} else {
		xp = NODE_GET_PARENT(y);
	}

	int is_left = 0;
	if (NODE_GET_PARENT(y) == NULL) {
		tree->root = x;
		xp = NULL;
	} else {
		struct node_st *yp = NODE_GET_PARENT(y);
		if (y == yp->left) {
			yp->left = x;
			is_left = 1;
		} else {
			yp->right = x;
			is_left = 0;
		}
	}

	int y_color = NODE_GET_COLOR(y);

	/* Swap in the node */
	if (y != node) {
		swap_node(tree, node, y);
		if (xp == node) {
			xp = y;
		}
	}

	if (y_color == COLOR_BLACK) {
		delete_rebalance(tree, x, xp, is_left);
	}

	node->parent = NULL;
	node->left = NULL;
	node->right = NULL;
}


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

inline static struct node_st *lowest_node(struct node_st *node)
{
	assert(node);
	while (node->left)
		node = node->left;
	return node;
}

inline static struct node_st *highest_node(struct node_st *node)
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
		struct node_st *parent = NODE_GET_PARENT(node);
		if (!parent)
			return NULL;
		while (node->parent && NODE_GET_PARENT(node)->left == node)
			node = NODE_GET_PARENT(node);
		if (node->parent && NODE_GET_PARENT(node)->right == node)
			node = NODE_GET_PARENT(node);
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
		struct node_st *parent = NODE_GET_PARENT(node);
		if (!parent)
			return NULL;
		while (node->parent && NODE_GET_PARENT(node)->right== node)
			node = NODE_GET_PARENT(node);
		if (node->parent && NODE_GET_PARENT(node)->left== node)
			node = NODE_GET_PARENT(node);
		else
			return NULL;
	}
	return node;
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

	ax_rb_cr self = AX_R_INIT(ax_one, it->owner);
	it->point = it->point ? lower_node(self.ax_map, it->point)
		: highest_node(self.ax_rb->root);
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_assert(it->point != NULL, "iterator boundary exceeded");
	ax_rb_cr self = AX_R_INIT(ax_one, it->owner);
	it->point =  higher_node(self.ax_map, it->point);
}

static bool citer_less(const ax_citer *it1, const ax_citer *it2)
{
	UNSUPPORTED();
	return false;
}

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);

	ax_rb_cr self = AX_R_INIT(ax_one, it1->owner);

	struct node_st *node1 = it1->point;
	struct node_st *node2 = it2->point;
	struct node_st *cur = lowest_node(self.ax_rb->root);

	size_t loc1, loc2;
	loc1 = loc2 = self.ax_rb->size;

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

	ax_rb_cr self = AX_R_INIT(ax_one, it->owner);
	it->point = it->point ? higher_node(self.ax_map, it->point)
		: lowest_node(self.ax_rb->root);
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr);

	ax_assert(it->point != NULL, "iterator boundary exceeded");
	ax_rb_cr self = AX_R_INIT(ax_one, it->owner);
	it->point =  lower_node(self.ax_map, it->point);
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

inline static void *node_set_value(ax_map *map, struct node_st *node, const void *val, va_list *ap, bool need_clean)
{
	ax_rb_r self = AX_R_INIT(ax_map, map);
	const ax_trait *vtr = ax_class_env(self.ax_box).elem_tr;
	ax_byte tmp[vtr->size];
	if (ax_trait_copy_or_init(vtr, tmp, val, ap))
		return NULL;

	void *ptr = node_val(map, node);
	if (need_clean)
		ax_trait_free(vtr, ptr);

	memcpy(ptr, tmp, vtr->size);
	return ptr;
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_VALIDITY(it, it->owner && it->point && it->tr);

	ax_rb_r self = AX_R_INIT(ax_map, it->owner);
	return !node_set_value(self.ax_map, it->point, val, ap, true);
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_NULL(it->point);

	ax_rb_r self = AX_R_INIT(ax_one, it->owner);
	struct node_st * node = it->point;

	struct node_st *next_node = ax_iter_norm(it)
		? higher_node(self.ax_map, node)
		: lower_node(self.ax_map, node);
	it->point = next_node;

	rb_tree_remove(self.ax_rb, node);
	self.ax_rb->size--;
	ax_trait_free(ax_class_env(self.ax_map).key_tr, node_key(node));
	ax_trait_free(ax_class_env(self.ax_box).elem_tr, node_val(self.ax_map, node));
	free(node);

}

static void *map_put(ax_map* map, const void *key, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(map);

	ax_rb_r self = AX_R_INIT(ax_map, map);

	const ax_trait *ktr = ax_class_env(self.ax_map).key_tr,
	      *vtr = ax_class_env(self.ax_box).elem_tr;

	struct node_st *node = malloc(sizeof(struct node_st) + ax_class_env(map).key_tr->size + vtr->size);
	if (node == NULL)
		return NULL;
	node->left = node->right = node->parent = NULL;

	struct node_st * candidate = NULL;
	char *valptr = NULL;
	switch (rb_tree_find_or_insert(self.ax_rb, key, node, &candidate)) {
		case 0:
			free(node);
			valptr = node_val(self.ax_map, candidate);
			if (node_set_value(map, candidate, val, ap, true))
				return NULL;
			break;
		case 1:
			valptr = node_val(self.ax_map, candidate);
			if (ax_trait_copy(ktr, candidate->kvbuffer, key)) {
				rb_tree_remove(self.ax_rb, candidate);
				return NULL;
			}

			if (!node_set_value(map, candidate, val, ap, false)) {
				rb_tree_remove(self.ax_rb, candidate);
				ax_trait_free(ktr, candidate->kvbuffer);
				return NULL;
			}
			self.ax_rb->size++;
			break;
	}

	return valptr;
}

static ax_fail map_erase(ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_rb_r self = AX_R_INIT(ax_map, map);
	struct node_st * node = rb_tree_find(self.ax_rb, key);
	if (!node)
		return true;

	const ax_trait *ktr = ax_class_env(self.ax_map).key_tr,
	      *vtr = ax_class_env(self.ax_box).elem_tr;

	rb_tree_remove(self.ax_rb, node);
	self.ax_rb->size--;
	ax_trait_free(ktr, node_key(node));
	ax_trait_free(vtr, node_val(self.ax_map, node));
	free(node);
	return false;
}

static void *map_get (const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_rb_cr self = AX_R_INIT(ax_map, map);
	struct node_st *node = rb_tree_find(self.ax_rb, key);
	if (!node)
		return NULL;
	return node_val(map, node);
}

static ax_iter map_at(const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_rb_cr self = AX_R_INIT(ax_map, map);

	struct node_st *node = rb_tree_find(self.ax_rb, key);
	if (!node)
		return box_end((ax_box *)self.ax_box);

	return (ax_iter) {
		.owner = (void *)map,
			.point = node,
			.tr = &ax_rb_tr.ax_box.iter,
			.etr = ax_class_env(self.ax_box).elem_tr,
	};
}

static bool map_exist(const ax_map* map, const void *key)
{
	CHECK_PARAM_NULL(map);
	ax_rb_cr self = AX_R_INIT(ax_map, map);
	struct node_st *node = rb_tree_find(self.ax_rb, key);
	return !!node;
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
	ax_rb_r self = AX_R_INIT(ax_one, one);
	box_clear(self.ax_box);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, ax_rb);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_map_cr self = AX_R_INIT(ax_any, any);
	return ax_map_dump(self.ax_map);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_rb_cr src = AX_R_INIT(ax_any, any);
	const ax_trait
		*ktr = ax_class_env(src.ax_map).key_tr,
		*vtr = ax_class_env(src.ax_box).elem_tr;
	ax_rb_r dst = { .ax_map = __ax_rb_construct(ktr, vtr)};

	ax_map_cforeach(src.ax_map, const void *, key, const void *, val) {
		if (!ax_map_put(dst.ax_map, key, val))
			goto fail;
	}

	return dst.ax_any;
fail:
	ax_one_free(dst.ax_one);
	return NULL;
}

static size_t box_size(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_rb_cr self = AX_R_INIT(ax_box, box);
	return self.ax_rb->size;
}

static size_t box_maxsize(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	return SIZE_MAX;
}

static ax_iter box_begin(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_rb_r self = AX_R_INIT(ax_box, box);
	struct node_st *node = self.ax_rb->root;
	if (node)
		while (node->left)
			node = node->left;
	ax_iter it = {
		.owner = box,
		.point = node,
		.tr = &ax_rb_tr.ax_box.iter,
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
		.tr = &ax_rb_tr.ax_box.iter,
		.etr = ax_class_env(box).elem_tr,
	};
	return it;
}

static ax_iter box_rbegin(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_rb_r self = AX_R_INIT(ax_box, box);
	struct node_st *node = self.ax_rb->root;
	if (node)
		while (node->right)
			node = node->right;
	ax_iter it = {
		.owner = box,
		.point = node,
		.tr = &ax_rb_tr.ax_box.riter,
		.etr = ax_class_env(self.ax_box).elem_tr,
	};
	return it;
}

static ax_iter box_rend(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = box,
		.tr = &ax_rb_tr.ax_box.riter,
		.point = NULL,
		.etr = ax_class_env(box).elem_tr,
	};
	return it;
}

static void box_clear(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_rb_r self = AX_R_INIT(ax_box, box);

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

	remove_child(self.ax_rb->root);
	self.ax_rb->root = NULL;
	self.ax_rb->size = 0;
}

const ax_map_trait ax_rb_tr =
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

ax_map *__ax_rb_construct(const ax_trait* key_tr, const ax_trait* val_tr)
{
	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(key_tr->less);
	CHECK_PARAM_NULL(key_tr->copy);
	CHECK_PARAM_NULL(key_tr->free);

	CHECK_PARAM_NULL(val_tr);
	CHECK_PARAM_NULL(val_tr->copy || val_tr->init);
	CHECK_PARAM_NULL(val_tr->free);

	ax_rb *rb = malloc(sizeof(ax_rb));
	if (rb == NULL)
		return NULL;

	ax_rb rb_init = {
		.ax_map = {
			.tr = &ax_rb_tr,
			.env.key_tr = key_tr,
			.env.ax_box.elem_tr = val_tr,
		},
		.size = 0,
		.rightmost = NULL,
		.root = NULL,
	};

	memcpy(rb, &rb_init, sizeof rb_init);
	return ax_r(ax_rb, rb).ax_map;
}

