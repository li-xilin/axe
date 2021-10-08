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

#ifndef AX_ARR_H
#define AX_ARR_H
#include "seq.h"

#define AX_ARR_NAME AX_SEQ_NAME ".arr"

#ifndef AX_ARR_DEFINED
#define AX_ARR_DEFINED
typedef struct ax_arr_st ax_arr;
#endif

#define AX_CLASS_BASE_arr seq
#define AX_CLASS_ROLE_arr(_l) _l AX_CLASS_PTR(arr); AX_CLASS_ROLE_seq(_l)
AX_CLASS_STRUCT_ROLE(arr);
AX_CLASS_STRUCT_ENTRY(arr)
	size_t size;
	void *arr;
AX_END;

extern const ax_seq_trait ax_arr_tr;

inline static void* ax_arr_ptr(ax_arr *arr)
{
	return arr->arr;
}

#define __ax_arr_construct3(_etr, _ptr, _size) \
	((struct ax_seq_st *)(struct ax_arr_st[1]) { \
		 	[0].seq.env.box.elem_tr = (_etr), \
			[0].seq.tr = &ax_arr_tr, \
			[0].size = (_size) / (_etr)->size, \
			[0].arr = (_ptr), \
		})

#define __ax_arr_construct2(_etr, _size) \
	__ax_arr_construct3(_etr, (uint8_t [(_size)]) { 0 }, _size)

#endif
