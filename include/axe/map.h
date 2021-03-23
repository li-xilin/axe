/*
  *Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
 *
  *Permission is hereby granted, free of charge, to any person obtaining a copy
  *of map software and associated documentation files (the "Software"), to deal
  *in the Software without restriction, including without limitation the rights
  *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  *copies of the Software, and to permit persons to whom the Software is
  *furnished to do so, subject to the following conditions:
 *
  *The above copyright notice and map permission notice shall be included in
  *all copies or substantial portions of the Software.
 *
  *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  *THE SOFTWARE.
 */

#ifndef AXE_MAP_H_
#define AXE_MAP_H_
#include "box.h"
#include "pair.h"
#include "def.h"

#define AX_MAP_NAME "one.any.box.map"

typedef struct ax_map_st ax_map;
typedef struct ax_map_trait_st ax_map_trait;

typedef ax_fail (*ax_map_put_f)  (ax_map *map, const void *key, const void *val);
typedef void  * (*ax_map_get_f)  (const ax_map *map, const void *key);
typedef ax_bool (*ax_map_exist_f)(const ax_map *map, const void *key);
typedef ax_fail (*ax_map_erase_f)(ax_map *map, const void *key);

struct ax_map_trait_st
{
	const ax_map_put_f put;
	const ax_map_get_f get;
	const ax_map_exist_f exist;
	const ax_map_erase_f erase;
};

struct ax_map_st
{
	ax_box _box;
	const ax_map_trait *const tr;
	const ax_stuff_trait *const key_tr;
	const ax_stuff_trait *const val_tr;
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


inline static ax_fail ax_map_put(ax_map *map, const void *key, const void *val)
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

inline static void *ax_map_cget(const ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->get);
	return map->tr->get(map, key);
}

inline static ax_bool ax_map_exist(const ax_map *map, const void *key)
{
	ax_trait_require(map, map->tr->exist);
	return map->tr->exist(map, key);
}

#endif
