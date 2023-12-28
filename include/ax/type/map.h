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
#include <stdarg.h>

#define ax_map_foreach(_map, _key_type, _key, _val_type, _val)            \
	for ( int _foreach_cont = 1; _foreach_cont; )                     \
	for (_key_type _key; _foreach_cont; _foreach_cont = 0)            \
	for (_val_type _val; _foreach_cont--; _foreach_cont = 0)          \
	_ax_box_iterate(ax_r(ax_map, _map).ax_box, _key##_cur, !_foreach_cont)  \
	for (_key = ax_map_iter_key(&_key##_cur),                         \
			_val = ax_iter_get(&_key##_cur),                  \
			_foreach_cont = 1;                                \
			_foreach_cont;                                    \
			_foreach_cont = 0)

#define ax_map_cforeach(_map, _key_type, _key, _val_type, _val)           \
	for ( int _foreach_cont = 1; _foreach_cont; )                     \
	for (_key_type _key; _foreach_cont; _foreach_cont = 0)            \
	for (_val_type _val; _foreach_cont--; _foreach_cont = 0)          \
	_ax_box_citerate(ax_cr(ax_map, _map).ax_box, _key##_cur, !_foreach_cont)\
	for (_key = ax_map_citer_key(&_key##_cur),                        \
			_val = ax_citer_get(&_key##_cur),                 \
			_foreach_cont = 1;                                \
			_foreach_cont;                                    \
			_foreach_cont = 0)

#ifndef AX_MAP_DEFINED
#define AX_MAP_DEFINED
typedef struct ax_map_st ax_map;
#endif

typedef ax_map *ax_map_init_f(const ax_trait* tr1, const ax_trait* tr2);

#define ax_baseof_ax_map ax_box

ax_abstract_code_begin(ax_map)
	void *(*put) (ax_map *map, const void *key, const void *val, va_list *ap);
	void *(*get) (const ax_map *map, const void *key);
	ax_iter (*at) (const ax_map *map, const void *key);
	bool (*exist) (const ax_map *map, const void *key);
	void *(*chkey) (ax_map *map, const void *key, const void *new_key);
	ax_fail (*erase) (ax_map *map, const void *key);
	const void *(*itkey)(const ax_citer *it);
ax_end;

ax_abstract_data_begin(ax_map)
	const ax_trait *key_tr;
ax_end;

ax_abstract_declare(3, ax_map);

inline static void *ax_map_put(ax_map *map, const void *key, const void *val)
{
	return ax_trait_out(map->env.ax_box.elem_tr, ax_obj_do(map, put, ax_trait_in(map->env.key_tr, key),
		ax_trait_in(map->env.ax_box.elem_tr, val), NULL));
}

inline static void *ax_map_iput(ax_map *map, const void *key, ...)
{
	va_list ap;
	va_start(ap, key);
	void *val = ax_obj_do(map, put, ax_trait_in(map->env.key_tr, key), NULL, &ap);
	va_end(ap);
	return ax_trait_out(map->env.ax_box.elem_tr, val);
}

inline static ax_fail ax_map_erase(ax_map *map, const void *key)
{
	return ax_obj_do(map, erase, ax_trait_in(map->env.key_tr, key));
}

inline static void *ax_map_get(ax_map *map, const void *key)
{
	return ax_trait_out(map->env.ax_box.elem_tr,
			ax_obj_do(map, get, ax_trait_in(map->env.key_tr, key)));
}

inline static ax_iter ax_map_at(ax_map *map, const void *key)
{
	return ax_obj_do(map, at, ax_trait_in(map->env.key_tr, key));
}

inline static ax_citer ax_map_cat(const ax_map *map, const void *key)
{
	return *ax_iter_c(ax_p(ax_iter, ax_obj_do(map, at, ax_trait_in(map->env.key_tr, key))));
}

inline static void *ax_map_cget(const ax_map *map, const void *key)
{
	return ax_obj_do(map, get, ax_trait_in(map->env.key_tr, key));
}

inline static bool ax_map_exist(const ax_map *map, const void *key)
{
	return ax_obj_do(map, exist, ax_trait_in(map->env.key_tr, key));
}

inline static void *ax_map_chkey(ax_map *map, const void *key, const void *new_key)
{
	return ax_obj_do(map, chkey, ax_trait_in(map->env.key_tr, key),
			ax_trait_in(map->env.key_tr, new_key));
}

inline static const void *ax_map_citer_key(const ax_citer *it)
{
	ax_obj_require((const ax_map *)it->owner, itkey);
	return ax_trait_out(it->etr, ax_class_trait((const ax_map *)it->owner).itkey(it));
}

inline static void *ax_map_iter_key(const ax_iter *it)
{
	ax_obj_require((const ax_map *)it->owner, itkey);
	return (void *)ax_trait_out(it->etr, ax_class_trait((const ax_map *)it->owner).itkey(ax_iter_cc(it)));
}

const void *ax_map_key(ax_map *map, const void *key);

ax_dump *ax_map_dump(const ax_map *map);

#endif
