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

#ifndef AX_MAP_H
#define AX_MAP_H
#include "box.h"
#include "def.h"

#define AX_MAP_NAME AX_BOX_NAME ".map"

#define ax_map_foreach(_map, _key_type, _key, _val_type, _val)                           \
	for ( int __ax_foreach_##_key##_flag = 1 ; __ax_foreach_##_key##_flag ; )        \
	for (_key_type _key; __ax_foreach_##_key##_flag ; )                              \
	for (_val_type _val; __ax_foreach_##_key##_flag ; __ax_foreach_##_key##_flag = 0)\
	ax_box_iterate(ax_r(map, _map).box, _key##_iter)                                 \
	for (	__ax_foreach_##_key##_flag = 1, _key = ax_map_iter_key(&_key##_iter),    \
      			_val = ax_iter_get(&_key##_iter);                                \
		__ax_foreach_##_key##_flag ;                                             \
		__ax_foreach_##_key##_flag = 0)

#define ax_map_cforeach(_map, _key_type, _key, _val_type, _val)                          \
	for ( int __ax_foreach_##_key##_flag = 1 ; __ax_foreach_##_key##_flag ; )        \
	for (_key_type _key; __ax_foreach_##_key##_flag ; )                              \
	for (_val_type _val; __ax_foreach_##_key##_flag ; __ax_foreach_##_key##_flag = 0)\
	ax_box_citerate(ax_cr(map, _map).box, _key##_iter)                               \
	for (	__ax_foreach_##_key##_flag = 1, _key = ax_map_citer_key(&_key##_iter),   \
      			_val = ax_citer_get(&_key##_iter);                               \
		__ax_foreach_##_key##_flag ;                                             \
		__ax_foreach_##_key##_flag = 0)


typedef struct ax_map_st ax_map;
typedef struct ax_map_trait_st ax_map_trait;

typedef void       *(*ax_map_put_f)   (ax_map *map, const void *key, const void *val);
typedef void       *(*ax_map_get_f)   (const ax_map *map, const void *key);
typedef ax_iter     (*ax_map_at_f)    (const ax_map *map, const void *key);
typedef bool     (*ax_map_exist_f) (const ax_map *map, const void *key);
typedef void       *(*ax_map_chkey_f) (ax_map *map, const void *key, const void *new_key);
typedef ax_fail     (*ax_map_erase_f) (ax_map *map, const void *key);
typedef const void *(*ax_map_it_key_f)(const ax_citer *it);

struct ax_map_trait_st
{
	const ax_box_trait box;
	const ax_map_put_f put;
	const ax_map_get_f get;
	const ax_map_at_f at;
	const ax_map_exist_f exist;
	const ax_map_chkey_f chkey;
	const ax_map_erase_f erase;
	const ax_map_it_key_f itkey;
};

typedef struct ax_map_env_st
{
	ax_one_env one;
	const ax_stuff_trait *key_tr;
	const ax_stuff_trait *val_tr;
} ax_map_env;

struct ax_map_st
{
	const ax_map_trait *const tr;
	ax_map_env env;
};

typedef union
{
	const ax_map *map;
	const ax_box *box;
	const ax_any *any;
	const ax_one *one;
} ax_map_cr;

typedef union
{
	ax_map *map;
	ax_box *box;
	ax_any *any;
	ax_one *one;
	ax_map_cr c;
} ax_map_r;


inline static void *ax_map_put(ax_map *map, const void *key, const void *val)
{
	ax_trait_require(map, map->tr->put);
	return map->tr->put(map, key, val);
}

inline static ax_fail ax_map_erase(ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->erase);
	return map->tr->erase(map, key);
}

inline static void *ax_map_get(ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->get);
	return map->tr->get(map, key);
}

inline static ax_iter ax_map_at(ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->at);
	return map->tr->at(map, key);
}

inline static ax_citer ax_map_cat(const ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->at);
	ax_iter it = map->tr->at(map, key);
	void *p = &it;
	return *(ax_citer *)p;
}

inline static void *ax_map_cget(const ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->get);
	return map->tr->get(map, key);
}

inline static bool ax_map_exist(const ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->exist);
	return map->tr->exist(map, key);
}

inline static void *ax_map_chkey(ax_map *map, const void *key, const void *new_key)
{
	ax_trait_require(map, map->tr->chkey);
	return map->tr->chkey(map, key, new_key);
}

inline static const void *ax_map_citer_key(ax_citer *it)
{
	return ((const ax_map *)it->owner)->tr->itkey(it);
}

inline static const void *ax_map_iter_key(ax_iter *it)
{
	return ((const ax_map *)it->owner)->tr->itkey(ax_iter_c(it));
}

#endif
