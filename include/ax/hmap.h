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
#include "type/map.h"

#ifndef AX_HMAP_DEFINED
#define AX_HMAP_DEFINED
typedef struct ax_hmap_st ax_hmap;
#endif

#define ax_baseof_ax_hmap ax_map
ax_concrete_declare(4, ax_hmap);

extern const ax_map_trait ax_hmap_tr;

ax_map *__ax_hmap_construct(
		const ax_trait* key_tr,
		const ax_trait* val_tr
);

inline static ax_concrete_creator(ax_hmap, const ax_trait* key_tr, const ax_trait* val_tr)
{
	return __ax_hmap_construct(key_tr, val_tr);
}

ax_fail ax_hmap_set_threshold(ax_hmap *hmap, size_t threshold);

size_t ax_hmap_threshold(ax_hmap *hmap);

ax_fail ax_hmap_rehash(ax_hmap *hmap, size_t size);

#endif

