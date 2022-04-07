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

#include "ax/hmap.h"
#include "ax/iter.h"
#include "ax/debug.h"
#include "ax/log.h"
#include "ax/trait.h"
#include "check.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>

#define DEFAULT_THRESHOLD 8

#define DEFINE_KVTR(map) \
	register const struct ax_trait_st \
		*ktr = (map)->env.key_tr,\
		*vtr = (map)->env.box.elem_tr

#undef free

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

ax_begin_data(hmap)
	size_t size;
	size_t buckets;
	size_t threshold;
	size_t reserved;
	struct bucket_st *bucket_list;
	struct bucket_st *bucket_tab;
ax_end;

static void    *map_put(ax_map *map, const void *key, const void *val, va_list *ap);
static ax_fail  map_erase(ax_map *map, const void *key);
static void    *map_get(const ax_map *map, const void *key);
static ax_iter  map_at(const ax_map *map, const void *key);
static bool     map_exist(const ax_map *map, const void *key);
static void    *map_chkey(ax_map *map, const void *key, const void *new_key);

static const void *map_it_key(const ax_citer *it);

static size_t   box_size(const ax_box *box);
static size_t   box_maxsize(const ax_box *box);
static ax_iter  box_begin(ax_box *box);
static ax_iter  box_end(ax_box *box);
static void     box_clear(ax_box *box);

static ax_dump *any_dump(const ax_any *any);
static ax_any  *any_copy(const ax_any *any);

static void     one_free(ax_one *one);
static const char *one_name(const ax_one *one);

static void     citer_next(ax_citer *it);
static void    *citer_get(const ax_citer *it);
static ax_fail  iter_set(const ax_iter *it, const void *p, va_list *ap);
static void     iter_erase(ax_iter *it);

static ax_fail rehash(ax_hmap *hmap, size_t new_size);
static struct node_st *make_node(ax_map *map, const void *key, const void *val, va_list *ap);
static struct bucket_st *locate_bucket(const ax_hmap *hmap, const void *key);
static void bucket_push_node(ax_hmap *hmap, struct bucket_st *bucket, struct node_st *node);
static struct bucket_st *unlink_bucket(struct bucket_st *head, struct bucket_st *bucket);
static struct node_st **find_node(const ax_map *map, struct bucket_st *bucket, const void *key);
static void free_node(ax_map *map, struct node_st **pp_node);
static void *value_set(ax_map* map, struct node_st *node, const void *val, va_list *ap);

/*
inline static int size_bit_find_last(size_t size, bool bit)
{
	if (!bit) {
		size = ~size;
		bit = !bit;
	}
	for (int i = sizeof size * 8 - 1; i >= 0; i--) {
		if (size & ((size_t)1 << i))
			return i;
	}
	return -1;
}

inline static void size_bit_set(size_t *size, bool bit, int len)
{
	size_t tmp = *size;
	assert(len <= sizeof(size_t) * 8);
	if (bit) {
		for (int i = 0; i < len; i++) {
			tmp |= 1 << i;
		}
	} else {
		for (int i = 0; i < len; i++) {
			tmp &= ~(1 << i);
		}
	}
	*size = tmp;
}
*/

static ax_fail rehash(ax_hmap *hmap, size_t nbucket)
{
	assert(nbucket > 0);
	struct bucket_st *new_tab = malloc((nbucket * sizeof(struct bucket_st)));
	if (!new_tab)
		return true;
	for (size_t i = 0; i != nbucket; i++)
		new_tab[i].node_list = NULL;

	struct bucket_st *bucket = hmap->bucket_list;
	hmap->bucket_list = NULL;
	for (; bucket; bucket = bucket->next) {
		for (struct node_st *currnode = bucket->node_list; currnode;) {
			const ax_trait *ktr = hmap->map.env.key_tr;

			struct bucket_st *new_bucket = new_tab
				+ ax_trait_hash(ktr, currnode->kvbuffer)
				% nbucket;

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
	free(hmap->bucket_tab);
	hmap->buckets = nbucket;
	hmap->bucket_tab = new_tab;
	return false;
}

ax_fail ax_hmap_rehash(ax_hmap *hmap, size_t nbucket)
{
	CHECK_PARAM_NULL(hmap);

	if (nbucket == 0) {
		size_t tmp = (hmap->size / hmap->threshold) + 1;
		if (tmp == nbucket)
			return false;
		nbucket = tmp;
	}

	if (nbucket * hmap->threshold < hmap->size) {
		errno = EDOM;
		return true;
	}

	return rehash(hmap, nbucket);
}

ax_fail ax_hmap_set_threshold(ax_hmap *hmap, size_t threshold)
{
	CHECK_PARAM_NULL(hmap);
	CHECK_PARAM_VALIDITY(threshold, threshold > 0);

	if (threshold * hmap->buckets < hmap->size) {
		size_t quo = hmap->size / threshold;
		size_t nbucket = quo ? hmap->size / threshold : 1;
		nbucket = ((box_maxsize(ax_r(hmap, hmap).box) >> 1) >= nbucket)
			? (nbucket << 1) | 1
			: box_maxsize(ax_r(hmap, hmap).box);

		if (rehash(hmap, nbucket))
			return true;
	}
	hmap->threshold = threshold;
	return false;
}

size_t ax_hmap_threshold(ax_hmap *hmap)
{
	return hmap->threshold;
}

static struct node_st *make_node(ax_map *map, const void *key, const void *val, va_list *ap)
{
	DEFINE_KVTR(map);
	size_t node_size = sizeof(struct node_st) + ktr->size + map->env.box.elem_tr->size;

	struct node_st *node = malloc(node_size);
	if (!node)
		return NULL;

	if (ax_trait_copy(ktr, node->kvbuffer, key))
		goto fail;

	if (ax_trait_copy_or_init(vtr, node->kvbuffer + ktr->size, val, ap))
		goto fail;
	return node;
fail:
	free(node);
	return NULL;
}

static inline struct bucket_st *locate_bucket(const ax_hmap *hmap, const void *key)
{
	size_t index = ax_trait_hash(hmap->map.env.key_tr, key) % hmap->buckets;
	return hmap->bucket_tab + index;
}

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
	const ax_trait *ktr = map->env.key_tr;
	for (pp_node = &bucket->node_list; *pp_node; pp_node = &((*pp_node)->next))
		if (ax_trait_equal(ktr, (*pp_node)->kvbuffer, key))
			return pp_node;
	return NULL;
}

static void free_node(ax_map *map, struct node_st **pp_node)
{
	assert(pp_node);
	DEFINE_KVTR(map);
	ax_byte *value_ptr = (*pp_node)->kvbuffer + map->env.key_tr->size;
	ax_trait_free(ktr, (*pp_node)->kvbuffer);
	ax_trait_free(vtr, value_ptr);
	struct node_st *node_to_free = *pp_node;
	(*pp_node) = node_to_free->next;
	free(node_to_free);
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	const ax_hmap *hmap= it->owner;
	struct node_st *node = it->point;
	struct bucket_st *bucket = locate_bucket(hmap, node->kvbuffer);
	assert(bucket);
	node = node->next;
	if (!node) {
		bucket = bucket->next;
		node = bucket ? bucket->node_list : NULL;
	}
	it->point = node;
}

static void *citer_get(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);

	ax_hmap_cr hmap_r = { .hmap = it->owner };
	struct node_st *node = it->point;
	const ax_trait *ktr = hmap_r.map->env.key_tr;
	return node->kvbuffer + ktr->size;
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->point);

	struct node_st *node = it->point;

	value_set(it->owner, node, val, ap);
	return false;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_NULL(it->point);


	ax_hmap_r hmap_r = { it->owner };
	struct node_st *node = it->point;
	void *pkey = node->kvbuffer;

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, pkey);
	struct node_st **findpp= find_node(hmap_r.map, bucket, pkey);
	ax_assert(findpp, "bad iterator");
	assert(it->point == *findpp);

	void *next = (*findpp)->next;
	if (!next) {
		if (bucket->next)
			next = bucket->next->node_list;

	}

	free_node(hmap_r.map, findpp);
	if (!bucket->node_list)
		hmap_r.hmap->bucket_list = unlink_bucket(hmap_r.hmap->bucket_list, bucket);
	hmap_r.hmap->size --;

	it->point = next;
}

inline static void *node_val(const ax_map* map, struct node_st *node)
{
	return node->kvbuffer + map->env.key_tr->size;
}

inline static void *node_key(struct node_st *node)
{
	return node->kvbuffer;
}

static void *value_set(ax_map* map, struct node_st *node, const void *val, va_list *ap)
{
	const ax_trait
		*ktr = map->env.key_tr,
		*vtr = map->env.box.elem_tr;

	ax_byte *value_ptr = node->kvbuffer + ktr->size;
	ax_byte tmp_buf[vtr->size];
	memcpy(tmp_buf, value_ptr, vtr->size);

	if (ax_trait_copy_or_init(vtr, value_ptr, val, ap)) {
		ax_trait_free(vtr, value_ptr);
		memcpy(value_ptr, tmp_buf, vtr->size);
		return NULL;
	}
	ax_trait_free(vtr, tmp_buf);
	return node_val(map, node);

}

static void *map_put(ax_map *map, const void *key, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(map);

	ax_hmap_r hmap_r = { .map = map };

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, key);
	struct node_st **findpp = find_node(hmap_r.map, bucket, key);
	if (findpp)
		return value_set(map, *findpp, val, ap);

	if (hmap_r.hmap->size == hmap_r.hmap->buckets * hmap_r.hmap->threshold) {
		if (hmap_r.hmap->buckets == ax_box_maxsize(ax_r(map, map).box)) {
			return NULL;
		}
		size_t new_size = hmap_r.hmap->buckets << 1 | 1;
		if(rehash(hmap_r.hmap, new_size))
			return NULL;
		bucket = locate_bucket(hmap_r.hmap, key);//bucket is invalid
	}
	struct node_st *new_node = make_node(hmap_r.map, key, val, ap);
	if (!new_node)
		return NULL;
	bucket_push_node(hmap_r.hmap, bucket, new_node);
	hmap_r.hmap->size ++;
	return node_val(map, new_node);
}

static ax_fail map_erase (ax_map *map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_hmap_r hmap_r = { .map = map };

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, key);
	struct node_st **findpp = find_node(hmap_r.map, bucket, key);
	if (!findpp)
		return true;

	free_node(map, findpp);
	if (!bucket->node_list)
		hmap_r.hmap->bucket_list = unlink_bucket(hmap_r.hmap->bucket_list, bucket);


	if (hmap_r.hmap->size <= (hmap_r.hmap->buckets >> 2) * hmap_r.hmap->threshold) {
		rehash(hmap_r.hmap, hmap_r.hmap->buckets >>= 1);
	}

	hmap_r.hmap->size --;

	return false;
}

static void *map_get (const ax_map *map, const void *key)
{
	CHECK_PARAM_NULL(map);

	const ax_hmap_cr hmap_r = { .map = map };

	if (!hmap_r.hmap->buckets)
		return NULL;

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, key);
	struct node_st **findpp = find_node(hmap_r.map, bucket, key);

	return findpp ? node_val(hmap_r.map, *findpp) : NULL;
}


static ax_iter  map_at(const ax_map *map, const void *key)
{
	CHECK_PARAM_NULL(map);

	const ax_hmap_cr hmap_r = { .map = map };

	if (!hmap_r.hmap->buckets)
		return box_end((ax_box *)hmap_r.box);

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, key);
	struct node_st **findpp = find_node(hmap_r.map, bucket, key);
	if (!findpp) {
		return box_end((ax_box *)hmap_r.box);
	}
	return (ax_iter) {
		.owner = (void *)map,
		.point = *findpp,
		.tr = &ax_hmap_tr.box.iter,
		.etr = map->env.box.elem_tr,
	};
}

static bool map_exist (const ax_map *map, const void *key)
{
	CHECK_PARAM_NULL(map);

	ax_hmap_r hmap_r = { .map = (ax_map*)map };
	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, key);
	struct node_st **findpp = find_node(hmap_r.map, bucket, key);
	if (findpp)
		return true;
	return false;
}

static void *map_chkey(ax_map *map, const void *key, const void *new_key)
{
	CHECK_PARAM_NULL(map);
	CHECK_PARAM_NULL(key);
	CHECK_PARAM_NULL(new_key);

	const ax_hmap_r hmap_r = { .map = map };

	const ax_trait *ktr = hmap_r.hmap->map.env.key_tr;

	if (!hmap_r.hmap->buckets)
		return NULL;

	struct bucket_st *bucket = locate_bucket(hmap_r.hmap, key);
	struct node_st **findpp = find_node(hmap_r.map, bucket, key);
	ax_assert(findpp, "key does not exists");

	struct node_st *node = *findpp;
	if (ax_trait_copy(ktr, node->kvbuffer, new_key))
		return NULL;

	(*findpp) = node->next;

	struct bucket_st *new_bucket = locate_bucket(hmap_r.hmap, new_key);
	struct node_st **destpp = find_node(hmap_r.map, new_bucket, new_key);
	if (destpp)
		free_node(hmap_r.map, destpp);

	bucket_push_node(hmap_r.hmap, new_bucket, node);
	return node_key(node);
}

static const void *map_it_key(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner && it->tr && it->point);
	CHECK_ITER_TYPE(it, one_name(NULL));
	struct node_st *node = it->point;
	const ax_map *map = it->owner;
	return ax_trait_out(map->env.key_tr, node_key(node));
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_hmap_r hmap_r= { .one = one };
	box_clear(hmap_r.box);
	free(hmap_r.hmap->bucket_tab);
	free(hmap_r.hmap);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, hmap);
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_map_cr self = { .any = any };
	return ax_map_dump(self.map);
}

static ax_any *any_copy(const ax_any *any)
{
	CHECK_PARAM_NULL(any);

	ax_hmap_r src_r = { .any = (ax_any *)any };
	const ax_trait *ktr = src_r.map->env.key_tr;
	const ax_trait *vtr = src_r.map->env.box.elem_tr;
	ax_hmap_r dst_r = { .map = __ax_hmap_construct(ktr, vtr)};
	ax_map_cforeach(src_r.map, const void *, key, const void *, val) {
		if (!ax_map_put(dst_r.map, key, val)) {
			ax_one_free(dst_r.one);
			return NULL;
		}
	}

	dst_r.hmap->map.env.box.any.one.scope.macro = NULL;
	dst_r.hmap->map.env.box.any.one.scope.micro = 0;
	return dst_r.any;
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

	return (~(size_t)0);
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_hmap_r hmap_r = { .box = box };

	ax_iter it = {
		.owner = (void *)box,
		.tr = &ax_hmap_tr.box.iter,
		.point = hmap_r.hmap->bucket_list
			? hmap_r.hmap->bucket_list->node_list
			: NULL,
		.etr = box->env.elem_tr,
	};
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);

	ax_iter it = {
		.owner = box,
		.tr = &ax_hmap_tr.box.iter,
		.point = NULL,
		.etr = box->env.elem_tr,
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

	void *new_bucket_tab = realloc(hmap_r.hmap->bucket_tab, sizeof(struct bucket_st));
	if (new_bucket_tab) {
		hmap_r.hmap->bucket_tab = new_bucket_tab;
	}

	hmap_r.hmap->bucket_tab->node_list = NULL;
	hmap_r.hmap->size = 0;
	hmap_r.hmap->bucket_list = NULL;
	hmap_r.hmap->buckets = 1;
}

const ax_map_trait ax_hmap_tr =
{
	.box = {
		.any = {
			.one = {
				.name  = one_name,
				.free  = one_free,
			},
			.dump = any_dump,
			.copy = any_copy,
		},
		
		.iter = {
			.norm  = true,
			.type  = AX_IT_FORW,
			.move = NULL,
			.prev = NULL,
			.next = citer_next,
			.less  = NULL,
			.dist  = NULL,
			.get   = citer_get,
			.set   = iter_set,
			.erase = iter_erase,
		},
		.size    = box_size,
		.maxsize = box_maxsize,
		.begin   = box_begin,
		.end     = box_end,
		.rbegin  = NULL,
		.rend    = NULL,
		.clear   = box_clear,
	},
	.put   = map_put,
	.get   = map_get,
	.at    = map_at,
	.erase = map_erase,
	.exist = map_exist,
	.chkey = map_chkey,
	.itkey = map_it_key,
};

ax_map *__ax_hmap_construct(const ax_trait *key_tr, const ax_trait *val_tr)
{
	CHECK_PARAM_NULL(key_tr);
	CHECK_PARAM_NULL(key_tr->equal);
	CHECK_PARAM_NULL(key_tr->hash);
	CHECK_PARAM_NULL(key_tr->copy);
	CHECK_PARAM_NULL(key_tr->free);

	CHECK_PARAM_NULL(val_tr);
	CHECK_PARAM_NULL(val_tr->copy || val_tr->init);
	CHECK_PARAM_NULL(val_tr->free);

	ax_hmap *hmap = malloc(sizeof(ax_hmap));
	if (!hmap)
		return NULL;
	
	ax_hmap hmap_init = {
		.map = {
			.tr = &ax_hmap_tr,
			.env = {
				.box.elem_tr = val_tr,
				.key_tr = key_tr,
			},
		},
		.buckets = 1,
		.size = 0,
		.threshold = DEFAULT_THRESHOLD,
		.bucket_tab = NULL,
		.bucket_list = NULL,
	};

	hmap_init.bucket_tab = malloc(sizeof(struct bucket_st) * hmap_init.buckets);
	if (!hmap_init.bucket_tab)
		return NULL;

	hmap_init.bucket_tab->node_list = NULL;
	memcpy(hmap, &hmap_init, sizeof hmap_init);
	return (ax_map *) hmap;
}

void dump_hmap(ax_hmap *hmap)
{
	for (struct bucket_st *b = hmap->bucket_list; b; b = b->next) {
		int c = 0;
		for (struct node_st *n = b->node_list; n; n = n->next) {
		ax_unused(n);
			c++;
		}
		printf("bucket %p %d\n", (void*)b, c);
	}
}

