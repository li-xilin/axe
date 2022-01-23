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

#ifndef AX_ARRAY_H
#define AX_ARRAY_H
#include "seq.h"
#include <limits.h>

#define AX_ARRAY_NAME AX_SEQ_NAME ".array"

#ifndef AX_ARRAY_DEFINED
#define AX_ARRAY_DEFINED
typedef struct ax_array_st ax_array;
#endif

#define AX_CLASS_BASE_array seq
#define AX_CLASS_ROLE_array(_l) _l AX_CLASS_PTR(array); AX_CLASS_ROLE_seq(_l)
AX_CLASS_STRUCT_ROLE(array);
AX_CLASS_STRUCT_ENTRY(array)
	size_t size;
	void *array;
AX_END;

extern const ax_seq_trait ax_array_tr;

inline static void* ax_array_ptr(ax_array *array)
{
	return array->array;
}

inline static void ax_array_reset(ax_array *array, char *ptr, size_t size)
{
	array->array = ptr;
	int elem_size = array->seq.env.box.elem_tr->size;
	array->size = elem_size ? size / elem_size : SIZE_MAX;
}

#define __ax_array_construct3(_etr, _ptr, _size) \
	((struct ax_seq_st *)(struct ax_array_st[1]) { \
		 	[0].seq.env.box.elem_tr = (_etr), \
			[0].seq.tr = &ax_array_tr, \
			[0].size = (_etr)->size ? (_size) / (_etr)->size : SIZE_MAX, \
			[0].array = (_ptr), \
		})

#define __ax_array_construct2(_etr, _size) \
	__ax_array_construct3(_etr, (uint8_t [(_size)]) { 0 }, _size)

#endif
