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

#include <ax/pool.h>
#include <ax/def.h>
#include <ax/debug.h>
#include <ax/base.h>
#include <ax/log.h>

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <errno.h>

#include "check.h"

#define STEP_SIZE sizeof(ax_fast_uint)

#define GROUP_BITS 4
#define BLOCK_BITS 11
#define NODE_BITS (STEP_SIZE*8 - GROUP_BITS - BLOCK_BITS)
#define MAXINT(_n) ((1UL<<((_n)))-1)
#define GROUP_MAX (MAXINT(GROUP_BITS)+1) + 1
#define BLOCK_MAX (MAXINT(BLOCK_BITS)+1)
#define NODE_MAX (MAXINT(NODE_BITS)+1)
#define BLOCKSIZE_MAX (GROUP_MAX * STEP_SIZE)

struct block
{
	struct node* node;
	ax_byte data[];
};

struct node
{
	struct group* group;
	intptr_t index;
	ax_byte *blocktab;
	ax_byte *freetab;
	struct node *pre;
	struct node *next;
	size_t blocktab_used;
	size_t freetab_used;
};

struct group
{
	intptr_t index;
	struct node** nodetab;
	struct node* avai_top;
	struct node* susp_top;
	size_t nodetab_size;
	size_t nodetab_used;
	size_t block_size;
};

struct ax_pool_st
{
	struct group* grouptab[GROUP_MAX];
};

inline static uint8_t
group_index(size_t size);

static struct group*
pool_prepare_group(ax_pool* pool, size_t size);

static struct block*
group_prepare_block(struct group* group);

static inline size_t
node_freed_size(struct node* node);

static struct node*
node_attach(struct node* header, struct node* node);

static struct node*
node_detach(struct node* header, struct node* node);

static void
suspend_node(struct group* group, struct node* node);

static bool
prepare_buffer(struct node* node);

static struct block*
node_pick_free_block(struct node* node);

static inline void*
block_ptr(struct node* node, uint16_t b);

static void 
block_free(struct block* block);

static struct node*
group_increase(struct group* group);

static void
group_decrease(struct group* group);

#ifdef AX_DEBUG
static inline bool
check_double_free(struct node *node, uint16_t shift);
#endif

void
ax_pool_dump(ax_pool* pool);

inline static uint8_t
group_index(size_t size)
{
	uint8_t shift = size / STEP_SIZE;
	int mod = size % STEP_SIZE;
	return shift - !mod;
}

static struct group*
pool_prepare_group(ax_pool* pool, size_t size)
{
	uint8_t shift = group_index(size);
	assert (shift < GROUP_MAX);

	struct group** group_ptr = pool->grouptab + shift;
	if (*group_ptr == NULL) {
		*group_ptr = malloc(sizeof(struct group));
		if(*group_ptr == NULL) {
			return NULL;
		}
		(*group_ptr)->index = shift;
		(*group_ptr)->nodetab= NULL;
		(*group_ptr)->nodetab_size= 0;
		(*group_ptr)->nodetab_used= 0;

		(*group_ptr)->avai_top= NULL;
		(*group_ptr)->susp_top= NULL;
		(*group_ptr)->block_size = sizeof(struct block) + (shift + 1) * STEP_SIZE;
	}

	return *group_ptr;
}

static struct node*
node_detach(struct node* header, struct node* node)
{
	assert(header || !node);
	if (node == NULL)
		return header;
	assert(header && node);

	if (node->next == node)
		return NULL;
	else {
		node->next->pre= node->pre;
		node->pre->next = node->next;
		return (node == header)
			? node->next
			: header;
	}
}

static struct node*
node_attach(struct node* header, struct node* node)
{
	if (node == NULL)
		return header;
	if (header == NULL)
		node->pre = node->next = node;
	else {
		struct node* pre = header->pre;
		header->pre = node;
		pre->next = node;
		node->pre = pre;
		node->next = header;
	}
	return node;
}

static void
suspend_node(struct group* group, struct node* node)
{
	group->avai_top = node_detach(group->avai_top, node);
	group->susp_top = node_attach(group->susp_top, node);
	free(node->blocktab);
	node->blocktab = NULL;
}

static ax_fail
prepare_buffer(struct node* node)
{
	assert(node);
	size_t blocktab_bsize = node->group->block_size * BLOCK_MAX;
	size_t freetab_bsize = sizeof(node->freetab[0]) * BLOCK_MAX;

	node->blocktab = malloc(blocktab_bsize + freetab_bsize);
	if (node->blocktab == NULL) {
		return true;
	}

	node->freetab = node->blocktab + blocktab_bsize;
	node->blocktab_used = 0;
	node->freetab_used = 0;
	return false;
}

static inline void*
block_ptr(struct node* node, uint16_t b)
{
	return node->blocktab + node->group->block_size * b;
}

static struct node*
group_increase(struct group* group)
{
	struct node* node = malloc(sizeof(struct node));
	if (node == NULL) {
			return NULL;
	}
	if (group->nodetab_size == group->nodetab_used) {
		if (group->nodetab_size == NODE_MAX) {
			return NULL;
		}
		size_t nodetab_size = group->nodetab_size ? (group->nodetab_size<<1) : 1;
		void * nodetab_ptr = realloc(group->nodetab, nodetab_size * sizeof(group->nodetab[0]));
		if (nodetab_ptr == NULL) {
			return NULL;
		}
		group->nodetab = nodetab_ptr;
		group->nodetab_size = nodetab_size;
	}
	group->nodetab[group->nodetab_used] = node;

	node->group = group;
	node->index = group->nodetab_used;

	if (prepare_buffer(node)) {
		free(node);
		return NULL;
	}
	group->nodetab_used ++;

	group->avai_top = node_attach(group->avai_top, node);
	return node;
}

static void
group_decrease(struct group* group)
{
	assert(group->nodetab_used);
	struct node* node = group->nodetab[group->nodetab_used - 1];
	assert(node->blocktab_used == node->freetab_used);
	group->nodetab_used --;
	assert(node->blocktab == NULL);
	group->susp_top = node_detach(group->susp_top, node);
	free(node);
	if (group->nodetab_used <= (group->nodetab_size >> 2))
	{
		group->nodetab = realloc(group->nodetab, (group->nodetab_size = group->nodetab_size >> 1) * sizeof(group->nodetab[0]));
	}
}

static struct block*
node_pick_free_block(struct node* node)
{
	assert(node);
	struct block* block = NULL;
	if (node->freetab_used) {
		node->freetab_used --;
		block = block_ptr(node, node->freetab[node->freetab_used]);
	} else if (node->blocktab_used != BLOCK_MAX) {
		block = block_ptr(node, node->blocktab_used);
		node->blocktab_used ++;
	}
	return block;
}

static struct block*
group_prepare_block(struct group* group)
{
	struct node* node;
	struct block* block;
	if ((node = group->avai_top) && (block = node_pick_free_block(node))) {
		if (node_freed_size(node) == BLOCK_MAX)
		{
			group->avai_top = node_detach(group->avai_top, node);
			group->avai_top = node_attach(group->avai_top, node);
			group->avai_top = group->avai_top->next;
		}
	} else {
		if (group->susp_top) {
			node = group->susp_top;
			group->susp_top = node_detach(group->susp_top, node);
			group->avai_top = node_attach(group->avai_top, node);
			if (prepare_buffer(node)) {
				return NULL;
			}
		} else {
			node = group_increase(group);
			if (node == NULL)
				return NULL;
		}
		node->blocktab_used = 1;
		block = (struct block*)node->blocktab;
	}
	block->node = node;

	return block;
}


static inline size_t
node_freed_size(struct node* node)
{
	return BLOCK_MAX - node->blocktab_used + node->freetab_used;
}

#ifdef AX_DEBUG
static bool
check_double_free(struct node *node, uint16_t shift)
{
	for (int i = 0; i < node->freetab_used; i++)
		if (node->freetab[i] == shift) {
			ax_pwarning("double free");
			return true;
		}
	return false;
}
#endif

static void
block_free(struct block* block)
{
	struct node* node = block->node;
	struct group* group = node->group;
	uint16_t shift = ((ax_byte*)block - node->blocktab) / node->group->block_size;

#ifdef AX_DEBUG
	if (check_double_free(node, shift))
		return;
#endif

	node->freetab[node->freetab_used] = shift;
	node->freetab_used ++;
	size_t avai_size = node_freed_size(node);
	if (avai_size == BLOCK_MAX) {
		suspend_node(group, node);
		while(group->nodetab_used && group->nodetab[group->nodetab_used - 1]->blocktab == NULL) 
			group_decrease(group);
	} else if (avai_size == 1) {
		group->avai_top = node_detach(group->avai_top, node);
		group->avai_top = node_attach(group->avai_top, node);
	}
}
#if 0
static inline size_t
node_buffer_size(struct node* node)
{
	return BLOCK_MAX * (node->group->block_size + sizeof(node->freetab[0]));
}
#endif

#if 1

void*
ax_pool_alloc(ax_pool* pool, size_t size)
{
	CHECK_PARAM_NULL(pool);

	if (size == 0)
		size = 1;

	struct block* block;
	if (size > BLOCKSIZE_MAX) {
		block = malloc(sizeof(struct block) + size);
		if (block == NULL) {
			return NULL;
		}
		block->node = NULL;
	} else {
		struct group* group = pool_prepare_group(pool, size);
		if (group == NULL)
			return NULL;
		block = group_prepare_block(group);
		if (block == NULL)
			return NULL;
	}
	return block->data;
}

void *
ax_pool_realloc(ax_pool* pool, void *ptr, size_t size)
{
	CHECK_PARAM_NULL(pool);

	if (!ptr)
		return ax_pool_alloc(pool, size);

	struct block* block = (struct block*)((ax_byte*)ptr - sizeof(struct block));

	if (block->node == NULL && size > BLOCKSIZE_MAX) {
		block = realloc(block, size + sizeof *block);
		return block ? block->data : NULL;
	}
	
	size_t old_size = block->node ? block->node->group->block_size : ~(size_t)0; // BUG
	size_t size_copy = AX_MIN(size, old_size);

	void *new = ax_pool_alloc(pool, size);
	if (!new)
		return NULL;

	memcpy(new, ptr, size_copy);
	ax_pool_free(ptr);
	return new;
}


void
ax_pool_free(void* ptr)
{
	if (ptr == NULL)
		return;

	struct block* block = (struct block*)((ax_byte*)ptr - sizeof(struct block));
	if (block->node == NULL)
		free(block);
	else
		block_free(block);
}
#else
void* 
ax_pool_alloc(ax_pool* pool, size_t size)
{
	return malloc(size);
}
void
ax_pool_free(void* ptr)
{
	free(ptr);
}

void *
ax_pool_realloc(ax_pool* pool, void* ptr, size_t size)
{
	return realloc(ptr, size);
}
#endif

void
ax_pool_dump(ax_pool* pool)
{
	CHECK_PARAM_NULL(pool);

	puts("--- POOL ---");

	size_t real_size = sizeof(pool->grouptab);
	size_t alloc_size = 0;

	for (uint8_t g = 0; g != GROUP_MAX; g++) {
		struct group* group = pool->grouptab[g];
		if (group) {
			printf("Group %-2hhu NodeUse:%zu/%zu MaxAlloc:%zuB\n", 
					g,
					group->nodetab_used,
					group->nodetab_size,
					group->block_size - sizeof(struct block));
			real_size += sizeof(struct group);
			for (size_t n = 0; n != group->nodetab_used; n++) {
				struct node *node = group->nodetab[n];
				printf("\tNode %-3zu Alloc:%-4zu Free:%-4zu\n", n, node->blocktab_used, node->freetab_used);
				if (node->blocktab) {
					real_size += node->group->block_size * BLOCK_MAX;
					real_size += sizeof(node->freetab[0]) * BLOCK_MAX;
					alloc_size += (node->group->block_size - sizeof(struct block)) * (node->blocktab_used - node->freetab_used);
				}
			}
		} else {
			printf("GROUP %-2hhu NOT USED\n", g);
		}
	}
	printf("Alloc/Real: %zu/%zu\n", alloc_size, real_size);
	puts("--- END POOL ---");
}

ax_pool*
ax_pool_create()
{
	ax_pool* pool;
	pool = malloc(sizeof(ax_pool));
	if (pool == NULL) {
		return NULL;
	}
	for (int i = 0; i < GROUP_MAX; i++) {
		pool->grouptab[i] = NULL;
	}
	return pool;
}

void
ax_pool_destroy(ax_pool* pool)
{
	if (!pool)
		return;

	for (uint8_t g = 0; g != GROUP_MAX; g++) {
		struct group* group = pool->grouptab[g];
		if (!group) continue;
		for (size_t n = 0; n != group->nodetab_used; n++) {
			struct node *node = group->nodetab[n];
			if (node->blocktab)
				free(node->blocktab);
			free(node);
		}
		free(group->nodetab);
		free(group);
	}
	free(pool);
}


