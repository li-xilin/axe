#ifndef AX_SPLAY_H
#define AX_SPLAY_H

#include <stddef.h>

typedef struct ax_splay_node_st ax_splay_node;

struct ax_splay_node_st
{
    ax_splay_node *parent, *left, *right;
};

typedef int ax_splay_cmp_f(const ax_splay_node *left, const ax_splay_node *right);

typedef struct ax_splay {
    ax_splay_node *root;
    ax_splay_cmp_f *comp;
    int size;
} ax_splay;

inline static void ax_splay_init(ax_splay* t, ax_splay_cmp_f *comp)
{
	t->comp = comp;
	t->root = NULL;
	t->size = 0;
}

ax_splay_node* ax_splay_insert(ax_splay *t, ax_splay_node *node);

ax_splay_node* ax_splay_find(ax_splay *t, const ax_splay_node *node);

ax_splay_node* ax_splay_first(ax_splay *t);

ax_splay_node* ax_splay_next(ax_splay_node *node);

ax_splay_node* ax_splay_prev(ax_splay_node *node);

ax_splay_node* ax_splay_last(ax_splay *t);

void ax_splay_remove(ax_splay *t, ax_splay_node *node);

void ax_splay_splay(ax_splay *t, ax_splay_node *x);

#endif
