#include "ax/splay.h"
#include <stdlib.h>
#include <assert.h>

static void check_sanity(ax_splay *t);
static void rotate(ax_splay_node *child);
static ax_splay_node* leftmost(ax_splay_node *node);
static ax_splay_node* rightmost(ax_splay_node *node);

inline static void zig(ax_splay_node *x, ax_splay_node *p);
inline static void zigzig(ax_splay_node *x, ax_splay_node *p);
inline static void zigzag(ax_splay_node *x, ax_splay_node *p);

/* This mutates the parental relationships, copy pointer to old parent. */
static void mark_gp(ax_splay_node *child);

void ax_splay_splay(ax_splay *t, ax_splay_node *x)
{
	while (1) {
		ax_splay_node *p = x->parent;
		if (!p) {
			t->root = x;
			return;
		}

		ax_splay_node *g = p->parent;
		if (!p->parent) {
			zig(x, p);
			continue;
		}
		if (x == p->left && p == g->left) {
			zigzig(x, p);
			continue;
		}
		if (x == p->right && p == g->right) {
			zigzig(x, p);
			continue;
		}
		zigzag(x, p);
	}
}

inline static void zig(ax_splay_node *x, ax_splay_node *p)
{
	rotate(x);
}

inline static void zigzig(ax_splay_node *x, ax_splay_node *p)
{
	rotate(p);
	rotate(x);
}

inline static void zigzag(ax_splay_node *x, ax_splay_node *p)
{
	rotate(x);
	rotate(x);
}

ax_splay_node *ax_splay_insert(ax_splay *t, ax_splay_node *new)
{
	ax_splay_node *removed = NULL;
	new->left = NULL;
	new->right = NULL;
	if (!t->root) {
		t->root = new;
		new->parent = NULL;
		goto out;
	}

	ax_splay_node *curr = t->root;
	ax_splay_node *parent;
	int left;
	while (curr) {
		parent = curr;
		int c = t->comp(new, curr);
		if (c < 0) {
			left = 1;
			curr = curr->left;
		}
		else if (c > 0) {
			left = 0;
			curr = curr->right;
		}
		else {
			new = removed = curr;
			goto out;
		}
	}
	new->parent = parent;
	if (left)
		parent->left = new;
	else
		parent->right = new;
out:
	ax_splay_splay(t, new);
	t->size++;
	return removed;
}

ax_splay_node* ax_splay_find(ax_splay *t, const ax_splay_node *node)
{
	ax_splay_node *curr = t->root;
	while (curr != NULL) {
		int c = t->comp(node, curr);
		if (c < 0)
			curr = curr->left;
		else if (c > 0)
			curr = curr->right;
		else
			goto found;
	}
	return NULL;
found:
	ax_splay_splay(t, curr);
	return curr;
}

void ax_splay_remove(ax_splay *t, ax_splay_node *node)
{
	if (!node)
		return;
	ax_splay_splay(t, node); /* Now node is t's root. */
	if (!node->left) {
		t->root = node->right;
		if (t->root != NULL)
			t->root->parent = NULL;
	}
	else if (!node->right) {
		t->root = node->left;
		t->root->parent = NULL;
	}
	else {
		ax_splay_node *x = leftmost(node->right);
		if (x->parent != node) {
			x->parent->left = x->right;
			if (x->right != NULL)
				x->right->parent = x->parent;
			x->right = node->right;
			x->right->parent = x;
		}
		t->root = x;
		x->parent = NULL;
		x->left = node->left;
		x->left->parent = x;
	}
	node->left = node->right = node->parent = NULL;
	t->size--;
	check_sanity(t);
}

ax_splay_node* ax_splay_first(ax_splay *t)
{
	return leftmost(t->root);
}

ax_splay_node* ax_splay_next(ax_splay_node *node)
{
	if (node->right)
		return leftmost(node->right);
	while (node->parent && node == node->parent->right)
		node = node->parent;
	return node->parent;
}

ax_splay_node* ax_splay_prev(ax_splay_node *node)
{
	if (node->left)
		return rightmost(node->left);
	while (node->parent && node == node->parent->left)
		node = node->parent;
	return node->parent;
}

ax_splay_node* ax_splay_last(ax_splay *t)
{
	return rightmost(t->root);
}

static void rotate(ax_splay_node *child)
{
	ax_splay_node *parent = child->parent;
	assert(parent != NULL);
	if (parent->left == child) {
		mark_gp(child);
		parent->left = child->right;
		if (child->right)
			child->right->parent = parent;
		child->right = parent;
	}
	else {
		mark_gp(child);
		parent->right = child->left;
		if (child->left)
			child->left->parent = parent;
		child->left = parent;
	}
}

static void mark_gp(ax_splay_node *child)
{
	ax_splay_node *parent = child->parent;
	ax_splay_node *grand = parent->parent;
	child->parent = grand;
	parent->parent = child;
	if (!grand)
		return;
	if (grand->left == parent)
		grand->left = child;
	else
		grand->right = child;
}

static ax_splay_node* leftmost(ax_splay_node *node)
{
	ax_splay_node *parent = NULL;
	while (node) {
		parent = node;
		node = node->left;
	}
	return parent;
}

static ax_splay_node* rightmost(ax_splay_node *node)
{
	ax_splay_node *parent = NULL;
	while (node) {
		parent = node;
		node = node->right;
	}
	return parent;
}

#ifndef NDEBUG
int check_node_sanity(ax_splay_node *x, void *floor, void *ceil, ax_splay_cmp_f *comp)
{
	int count = 1;
	if (x->left) {
		assert(x->left->parent == x);
		void *new_floor;
		if (!floor || comp(x, floor) < 0)
			new_floor = x;
		else
			new_floor = floor;
		count += check_node_sanity(x->left, new_floor, ceil, comp);
	}
	if (x->right) {
		assert(x->right->parent == x);
		void *new_ceil;
		if (!ceil || comp(x, ceil) > 0)
			new_ceil = x;
		else
			new_ceil = ceil;
		count += check_node_sanity(x->right, floor, new_ceil, comp);
	}
	return count;
}
#endif

static void check_sanity(ax_splay *t)
{
#ifdef DEBUG
	if (!t->root)
		assert(t->size == 0);
	else {
		int reachable = check_node_sanity(t->root, NULL, NULL, t->comp);
		assert(reachable == t->size);
	}
#endif
}
