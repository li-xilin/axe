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

#ifndef AXE_AVLTREE_H_
#define AXE_AVLTREE_H_
#include "map.h"

typedef struct ax_avltree_st ax_avltree;

typedef union
{
	const ax_avltree *avltree;
	const ax_map *map;
	const ax_box *box;
	const ax_any *any;
	const ax_one *one;
} ax_avltree_crol;

typedef union
{
	ax_avltree *avltree;
	ax_map *map;
	ax_box *box;
	ax_any *any;
	ax_one *one;
	ax_avltree_crol c;
} ax_avltree_role;

ax_map *__ax_avltree_construct(ax_base* base,
		const ax_stuff_trait* key_tr,
		const ax_stuff_trait* val_tr);

ax_avltree_role ax_avltree_create(ax_scope *scope,
		const ax_stuff_trait *key_tr,
		const ax_stuff_trait *val_tr);

#endif

