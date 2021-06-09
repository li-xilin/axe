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

#ifndef AX_HMAP_H
#define AX_HMAP_H
#include "map.h"

#define AX_HMAP_NAME AX_MAP_NAME ".hmap"

#ifndef AX_HMAP_DEFINED
#define AX_HMAP_DEFINED
typedef struct ax_hmap_st ax_hmap;
#endif

typedef union
{
	const ax_hmap *hmap;
	const ax_map *map;
	const ax_box *box;
	const ax_any *any;
	const ax_one *one;
} ax_hmap_cr;

typedef union
{
	ax_hmap *hmap;
	ax_map *map;
	ax_box *box;
	ax_any *any;
	ax_one *one;
	ax_hmap_cr c;
} ax_hmap_r;

extern const ax_map_trait ax_hmap_tr;

ax_map *__ax_hmap_construct(
		const ax_stuff_trait* key_tr,
		const ax_stuff_trait* val_tr
);

ax_hmap_r ax_hmap_create(
		ax_scope *scope,
		const ax_stuff_trait *key_tr,
		const ax_stuff_trait *val_tr
);


ax_fail ax_hmap_rehash(ax_hmap *hmap, size_t size);

#endif

