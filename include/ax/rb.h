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

#ifndef AX_RB_H
#define AX_RB_H
#include "class.h"
#include "map.h"

#define AX_RB_NAME AX_MAP_NAME ".rb"

#ifndef AX_RB_DEFINED
#define AX_RB_DEFINED
typedef struct ax_rb_st ax_rb;
#endif

extern const ax_map_trait ax_rb_tr;

#define ax_baseof_rb map

ax_role(4, rb);

extern const ax_map_trait ax_hmap_tr;

ax_map *__ax_rb_construct(
		const ax_trait* key_tr,
		const ax_trait* val_tr
);

inline static ax_class_constructor(rb, const ax_trait* key_tr, const ax_trait* val_tr)
{
	return __ax_rb_construct(key_tr, val_tr);
}

#endif

