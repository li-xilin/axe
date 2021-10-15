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

#ifndef AX_AVL_H
#define AX_AVL_H
#include "map.h"

#define AX_AVL_NAME AX_MAP_NAME ".avl"

#ifndef AX_AVL_DEFINED
#define AX_AVL_DEFINED
typedef struct ax_avl_st ax_avl;
#endif

extern const ax_map_trait ax_avl_tr;

#define AX_CLASS_BASE_avl map
#define AX_CLASS_ROLE_avl(_l) _l AX_CLASS_PTR(avl); AX_CLASS_ROLE_map(_l)

AX_CLASS_STRUCT_ROLE(avl);

extern const ax_map_trait ax_hmap_tr;

ax_map *__ax_avl_construct(
		const ax_trait* key_tr,
		const ax_trait* val_tr
);

inline static AX_CLASS_CONSTRUCTOR(avl, const ax_trait* key_tr, const ax_trait* val_tr)
{
	return __ax_avl_construct(key_tr, val_tr);
}

#endif

