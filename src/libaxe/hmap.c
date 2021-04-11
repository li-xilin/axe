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

#include <axe/hmap.h>
#include <axe/map.h>
#include <axe/iter.h>
#include <axe/scope.h>
#include <axe/pool.h>
#include <axe/debug.h>
#include <axe/base.h>
#include <axe/error.h>
#include <axe/log.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "check.h"

#undef free

#define REALLOC_THRESHOLD 1

struct node_st
{
	struct node_st *next;
	ax_byte kvbuffer[];
};

struct bucket_st
{
	struct node_st *node_list;
	struct bucket_st *prev;
	struct bucket_st *next;
};

struct ax_hmap_st
{
	ax_map _map;
	size_t size;
	size_t capacity;
	struct bucket_st *bucket_list;
	struct bucket_st *bucket_tab;
};

static ax_bool  map_put(ax_map *map, const void *key, const void *val);
static ax_bool  map_erase(ax_map *map, const void *key);
static void    *map_get(const ax_map *map, const void *key);
static ax_bool  map_exist(const ax_map *map, const void *key);
static const void *map_it_key(ax_citer *it);

static size_t   box_size(const ax_box *box);
static size_t   box_maxsize(const ax_box *box);
static ax_iter  box_begin(ax_box *box);
static ax_iter  box_end(ax_box *box);
static void     box_clear(ax_box *box);
static const ax_stuff_trait *box_elem_tr(const ax_box *box);

static void     any_dump(const ax_any *any, int ind);
static ax_any  *any_copy(const ax_any *any);
static ax_any  *any_move(ax_any *any);

static void     one_free(ax_one *one);

static void     citer_next(ax_citer *it);
static void    *iter_get(const ax_iter *it);
static ax_fail  iter_set(const ax_iter *it, const void *p);
static void     iter_erase(ax_iter *it);

static ax_fail rehash(ax_hmap *hmap, size_t new_size);
static struct node_st *make_node(ax_map *map, const void *key, const void *val);
static struct bucket_st *locate_bucket(const ax_hmap *hmap, const void *key);
static void bucket_push_node(ax_hmap *hmap, struct bucket_st *bucket, struct node_st *node);
static struct bucket_st *unlink_bucket(struct bucket_st *head, struct bucket_st *bucket);
static struct node_st **find_node(const ax_map *map, struct bucket_st *bucket, const void *key);
static void free_node(ax_map *map, struct node_st **pp_node);

static const ax_map_trait map_trait;

static ax_fail rehash(ax_hmap *hmap, size_t new_size)
{
	ax_hmap_r hmap_r = { hmap };
	ax_base *base = ax_one_base(hmap_r.one);
	ax_pool *pool = ax_base_pool(base);

	struct bucket_st *new_tab = ax_pool_alloc(pool, (new_size * sizeof(struct bucket_st)));
	if (!new_tab) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}
	for (size_t i = 0; i != new_size; i++)
		new_tab[i].node_list = NULL;

	struct bucket_st *bucket = hmap->bucket_list;
	hmap->bucket_list = NULL; //re-link
	for (;bucket; bucket = bucket->next) {
		for (struct node_st *currnode = bucket->node_list; currnode;) {
			struct bucket_st *new_bucket = new_tab
				+ hmap->_map.env.key_tr->hash(currnode->kvbuffer, hmap->_map.env.key_tr->size)
				% new_size;
			if (!new_bucket->node_list) {
				new_bucket->next = hmap->bucket_list;
				new_bucket->prev = NULL;
				if (hmap->bucket_list)
					hmap->bucket_list->prev = new_bucket;
				hmap->bucket_list = new_bucket;
			}

			struct node_st *old_head = new_bucket->node_list, * node = currnode;
			currnode = currnode->next;
			node->next = old_head;
			new_bucket->node_list = node;
		}
		bucket->node_list = NULL;
	}
	ax_pool_free(hmap->bucket_tab);
	hmap->capacity = new_size;
	hmap->bucket_tab = new_tab;
	return ax_false;
}

static struct node_st *make_node(ax_map *map, const void *key, const void *val)
{
	ax_hmap_r hmap_r = { .map = map };

	ax_base *base = ax_one_base(hmap_r.one);
	ax_pool *pool = ax_base_pool(base);

	size_t key_size = map->env.key_tr->size;
	size_t node_size = sizeof(struct node_st) + key_size + map->env.val_tr->size;

	struct node_st *node = ax_pool_alloc(pool, node_size);
	if (!node) {
		ax_base_set_errno(base, AX_ERR_NOKEY);
		return NULL;
	}
	map->env.key_tr->copy(pool, node->kvbuffer, key, map->env.key_tr->size);
	map->env.val_tr->copy(pool, node->kvbuffer + key_size, val, map->env.val_tr->size);
	return node;
}

static inline struct bucket_st *locate_bucket(const ax_hmap *hmap, const void *key)
{
	size_t index = hmap->_map.env.key_tr->hash(key, hmap->_map.env.key_tr->size) % hmap->capacity;
	return hmap->bucket_tab + index;
}
#if 0
static void bucket_push_node(ax_hmap *hmap, struct bucket_st *bucket, struct node_st *node)
{
	if (!bucket->node_list) {
		if (hmap->bucket_list)
			hmap->bucket_list->prev = bucket;
		bucket->next = hmap->bucket_list;
		hmap->bucket_list = bucket;
	}
	node->next = bucket->node_list;
	bucket->node_list = node;
}
#else 

static void bucket_push_node(ax_hmap *hmap, struct bucket_st *bucket, struct node_st *node)
{
	if (!bucket->node_list) {
		if (hmap->bucket_list)
			hmap->bucket_list->prev = bucket;
		bucket->prev = NULL;
		bucket->next = hmap->bucket_list;
		hmap->bucket_list = bucket;
	}
	node->next = bucket->node_list;
	bucket->node_list = node;
}
#endif


static struct bucket_st *unlink_bucket(struct bucket_st *head, struct bucket_st *bucket)
{
	assert(head);
	assert(bucket);
	struct bucket_st *ret;
	ret = head == bucket ? bucket->next : head;

	if (bucket->prev)
		bucket->prev->next = bucket->next;
	if (bucket->next)
		bucket->next->prev = bucket->prev;
	return ret;
}

static struct node_st **find_node(const ax_map *map, struct bucket_st *bucket, const void *key)
{
	struct node_st **pp_node;
	for (pp_node = &bucket->node_list; *pp_node; pp_node = &((*pp_node)->next))
		if (map->env.key_tr->equal((*pp_node)->kvbuffer, key, map->env.key_tr->size))
			return pp_node;
	return NULL;
}

static void free_node(ax_map *map, struct node_st **pp_node)
{
	assert(pp_node);
	ax_byte *value_ptr = (*pp_node)->kvbuffer + map->env.key_tr->size;
	map->env.key_tr->free((*pp_node)->kvbuffer);
	map->env.val_tr->free(value_ptr);
	struct node_st *node_to_free = *pp_node;
	(*pp_node) = node_to_free->next;
	ax_pool_free(node_to_free);
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	const ax_hmap *hmap= it->owner;
	struct node_st **pp_node = it->point;
	struct bucket_st *bucket = locate_bucket(hmap, (*pp_node)->kvbuffer);
	assert(bucket);
	pp_node = &(*pp_node)->next;
	if (!*pp_node) {
		bucket = bucket->next;
		pp_node = bucket ? &bucket->node_list : NULL;
	}
	it->point = pp_node;
}

static void *iter_get(const ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_hmap_cr hmap_r = { .hmap = it->owner };
	struct node_st **pp_node = it->point;

	const ax_stuff_trait
		*key_tr = hmap_r.map->env.key_tr,
		*val_tr = hmap_r.map->env.val_tr;

	void *pval = (*pp_node)->kvbuffer + key_tr->size;

	void *val = (val_tr->link)
		? *(void**)pval
		: pval;

	return val;
}

static ax_fail iter_set(const ax_iter *it, const void *p)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->point);

	struct node_st **pp_node = it->point;

	ax_hmap_r hmap_r = { it->owner };
	ax_hmap *hmap= (ax_hmap*)it->owner;
	assert(hmap->size != 0);

	ax_base *base = ax_one_base(hmap_r.one);
	ax_pool *pool = ax_base_pool(base);
	const ax_stuff_trait *val_tr = hmap_r.map->env.val_tr;

	val_tr->free((*pp_node)->kvbuffer + hmap_r.map->env.key_tr->size);
	if (val_tr->copy(pool, (*pp_node)->kvbuffer + hmap_r.map->env.key_tr->size,
				p, hmap_r.map->env.val_tr->size)) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}
	return ax_false;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_NULL(it->point);

	ax_hmap_r hmap_r = { it->point };
	struct node_st **pp_node = it->point;
	void *pkey = (*pp_node)->kvbuffer;

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, pkey);
	struct node_st **pp_find_result = find_node(hmap_r.map, bucket, pkey);
	ax_assert(pp_find_result, "bad iterator");

	free_node(hmap_r.map, pp_find_result);
	struct node_st **pp_next = (*pp_find_result)
		? pp_find_result
		: (bucket->next
			? &bucket->next->node_list
			: NULL);

	if (!bucket->node_list)
		hmap_r.hmap->bucket_list = unlink_bucket(hmap_r.hmap->bucket_list, bucket);
	hmap_r.hmap->size --;

	if (!pp_node)
		rehash(hmap_r.hmap, 0);

	it->point = pp_next;
}

static ax_fail map_put (ax_map *map, const void *key, const void *val)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);

	ax_hmap_r hmap_r = { .map = map };
	ax_base *base  = ax_one_base(hmap_r.one);
	ax_pool *pool = ax_base_pool(base);

	const void *pkey = map->env.key_tr->link ? &key : key;
	const void *pval = map->env.val_tr->link ? &val : val;

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, pkey);
	struct node_st **pp_find_result = find_node(hmap_r.map, bucket, pkey);
	if (pp_find_result) {
		ax_byte *value_ptr = (*pp_find_result)->kvbuffer + map->env.key_tr->size;
		map->env.val_tr->free(value_ptr);
		map->env.val_tr->copy(pool, value_ptr, pval, map->env.val_tr->size);
	} else {
		if (hmap_r.hmap->size == hmap_r.hmap->capacity * REALLOC_THRESHOLD) {
			if (hmap_r.hmap->capacity == ax_box_maxsize(ax_r(map, map).box)) {
				ax_base_set_errno(base, AX_ERR_FULL);
				return ax_true;
			}
			size_t new_size = hmap_r.hmap->capacity << 1 | 1;
			if(rehash(hmap_r.hmap, new_size))
				return ax_true;
			bucket = locate_bucket(hmap_r.hmap, pkey);//bucket is invalid
		}
		struct node_st *new_node = make_node(hmap_r.map, pkey, pval);
		if (!new_node)
			return ax_true;
		bucket_push_node(hmap_r.hmap, bucket, new_node);
		hmap_r.hmap->size ++;
	}
	return ax_false;
}

static ax_fail map_erase (ax_map *map, const void *key)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);

	ax_hmap_r hmap_r = { .map = map };
	const void *pkey = map->env.key_tr->link ? &key : key;
	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, pkey);
	struct node_st **pp_find_result = find_node(hmap_r.map, bucket, pkey);
	if (!pp_find_result) {
		ax_base *base = ax_one_base(hmap_r.one);
		ax_base_set_errno(base, AX_ERR_NOKEY);
		return ax_true;
	}

	free_node(map, pp_find_result);
	if (!bucket->node_list)
		hmap_r.hmap->bucket_list = unlink_bucket(hmap_r.hmap->bucket_list, bucket);
	hmap_r.hmap->size --;
	if (hmap_r.hmap->size <= (hmap_r.hmap->capacity >> 2) * REALLOC_THRESHOLD)
		if (rehash(hmap_r.hmap, hmap_r.hmap->capacity >>= 1)) {
			ax_pwarning("Hashmap rehash failed");
		}

	return ax_false;
}

static void *map_get (const ax_map *map, const void *key)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);

	const ax_hmap_cr hmap_r = { .map = map };
	const void *pkey = map->env.key_tr->link ? &key : key;

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, pkey);
	struct node_st **pp_find_result = find_node(hmap_r.map, bucket, pkey);
	if (!pp_find_result) {
		ax_base *base = ax_one_base(hmap_r.one);
		ax_base_set_errno(base, AX_ERR_NOKEY);
		return NULL;
	}
	return  (*pp_find_result)->kvbuffer + hmap_r.map->env.key_tr->size;
}

static ax_bool map_exist (const ax_map *map, const void *key)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);

	ax_hmap_r hmap_r = { .map = (ax_map*)map };
	const void *pkey = map->env.key_tr->link ? &key : key;
	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, pkey);
	struct node_st **pp_find_result = find_node(hmap_r.map, bucket, pkey);
	if (pp_find_result)
		return ax_true;
	return ax_false;
}

static const void *map_it_key(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_hmap_cr hmap_r = { .hmap = it->owner };
	struct node_st **pp_node = it->point;

	const ax_stuff_trait *key_tr = hmap_r.map->env.key_tr;

	return (key_tr->link)
		? *(void**)(*pp_node)->kvbuffer
		: (*pp_node)->kvbuffer;
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_hmap_r hmap_r= { .one = one };
	ax_scope_detach(hmap_r.one);
	box_clear(hmap_r.box);
	ax_pool_free(hmap_r.hmap->bucket_tab);
	ax_pool_free(hmap_r.hmap);
}

static void any_dump(const ax_any *any, int ind)
{
	CHECK_PARAM_NULL(any);

	ax_pinfo("have not implemented");
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_hmap_r src_r = { .any = (ax_any *)any };
	ax_base *base = ax_one_base(src_r.one);
	const ax_stuff_trait *ktr = src_r.map->env.key_tr;
	const ax_stuff_trait *vtr = src_r.map->env.val_tr;
	ax_hmap_r dst_r = { .map = __ax_hmap_construct(base, ktr, vtr)};
	ax_map_cforeach(src_r.map, const void *, key, const void *, val) {
		if (ax_map_put(dst_r.map, key, val)) {
			ax_one_free(dst_r.one);
			return NULL;
		}
	}

	dst_r.hmap->_map.env.one.scope = NULL;
	dst_r.hmap->_map.env.one.sindex = 0;
	ax_scope_attach(ax_base_local(base), dst_r.one);

	return dst_r.any;
}

static ax_any *any_move(ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_hmap_r src_r = { .any = any };
	ax_base *base = ax_one_base(src_r.one);
	ax_pool *pool = ax_base_pool(base);

	ax_hmap *dst = ax_pool_alloc(pool, sizeof(ax_hmap));
	memcpy(dst, src_r.hmap, sizeof(ax_hmap));
	src_r.hmap->capacity = 0;
	src_r.hmap->size = 0;


	dst->_map.env.one.scope = NULL;
	dst->_map.env.one.sindex = 0;
	ax_scope_attach(ax_base_local(base), ax_r(hmap, dst).one);

	return (ax_any *) dst;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_hmap_r hmap_r = { .box = (ax_box*)box };
	return hmap_r.hmap->size;
}

static size_t box_maxsize(const ax_box *box)
{
	CHECK_PARAM_NULL(box);

	return (~(size_t)0) >> 1;
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_hmap_r hmap_r = { .box = box };
	struct node_st **pp_node;
	pp_node = &hmap_r.hmap->bucket_list->node_list;

	ax_iter it = {
		.owner = (void *)box,
		.tr = &map_trait.box.iter,
		.point = pp_node ? pp_node : NULL
	};
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = box,
		.tr = &map_trait.box.iter,
		.point = NULL
	};
	return it;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_hmap_r hmap_r = { .box = (ax_box*)box };
	for (struct bucket_st *bucket = hmap_r.hmap->bucket_list; bucket; bucket = bucket->next)
		for (struct node_st **pp_node = &bucket->node_list; *pp_node;)
			free_node(hmap_r.map, pp_node);

	ax_pool *pool = ax_base_pool(ax_one_base(hmap_r.one));
	void *new_bucket_tab = ax_pool_alloc(pool, sizeof(struct bucket_st));
	if (new_bucket_tab) {
		ax_pool_free(hmap_r.hmap->bucket_tab);
		hmap_r.hmap->bucket_tab = new_bucket_tab;
	}
	hmap_r.hmap->bucket_tab->node_list = NULL;
	hmap_r.hmap->size = 0;
	hmap_r.hmap->bucket_list = NULL;
	hmap_r.hmap->capacity = 1;
}

static const ax_stuff_trait *box_elem_tr(const ax_box *box)
{
	ax_hmap_r hmap_r = { .box = (ax_box*)box };
	return hmap_r.map->env.val_tr;
}

static const ax_map_trait map_trait =
{
	.box = {
		.any = {
			.one = {
				.name  = AX_HMAP_NAME,
				.free  = one_free,
			},
			.dump = any_dump,
			.copy = any_copy,
			.move = any_move,
		},
		
		.iter = {
			.ctr = {
				.norm  = ax_true,
				.type  = AX_IT_FORW,
				.move = NULL,
				.prev = NULL,
				.next = citer_next,
				.less  = NULL,
				.dist  = NULL,
			},
			.get   = iter_get,
			.set   = iter_set,
			.erase = iter_erase,
		},
		.riter = { { NULL } },

		.size    = box_size,
		.maxsize = box_maxsize,
		.begin   = box_begin,
		.end     = box_end,
		.rbegin  = NULL,
		.rend    = NULL,
		.clear   = box_clear,
		.elem_tr = box_elem_tr
	},
	.put   = map_put,
	.get   = map_get,
	.erase = map_erase,
	.exist = map_exist,
	.itkey = map_it_key
};

ax_map *__ax_hmap_construct(ax_base *base, const ax_stuff_trait *key_tr, const ax_stuff_trait *val_tr)
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


	ax_hmap *hmap = ax_pool_alloc(ax_base_pool(base), sizeof(ax_hmap));
	if (!hmap) {
		ax_base_set_errno(base, AX_ERR_NOKEY);
		return NULL;
	}
	
	ax_hmap hmap_init = {
		._map = {
			.tr = &map_trait,
			.env = {
				.one = {
					.base = base,
					.scope = NULL,
					.sindex = 0
				},
				.key_tr = key_tr,
				.val_tr = val_tr,
			},
		},
		.capacity = 1,
		.size = 0,
		.bucket_tab = NULL,
		.bucket_list = NULL,
	};

	hmap_init.bucket_tab = ax_pool_alloc(ax_base_pool(base), sizeof(struct bucket_st) * hmap_init.capacity);
	if (!hmap_init.bucket_tab) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}
	hmap_init.bucket_tab->node_list = NULL;
	memcpy(hmap, &hmap_init, sizeof hmap_init);
	return (ax_map *) hmap;
}

ax_hmap_r ax_hmap_create(ax_scope *scope, const ax_stuff_trait *key_tr, const ax_stuff_trait *val_tr)
{
	CHECK_PARAM_NULL(scope);
	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(val_tr);

	ax_base *base = ax_one_base(ax_r(scope, scope).one);
	ax_hmap_r hmap_r =  { .map = __ax_hmap_construct(base, key_tr, val_tr) };
	if (!hmap_r.one)
		return hmap_r;
	ax_scope_attach(scope, hmap_r.one);
	return hmap_r;
}

