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

typedef struct ax_arr_st 
{
	ax_seq seq;
	size_t size;
	void *arr;
} ax_arr;

typedef union
{
	const ax_arr* arr;
	const ax_seq* seq;
	const ax_box* box;
	const ax_any* any;
	const ax_one* one;
} ax_arr_cr;

typedef union
{
	ax_arr* arr;
	ax_seq* seq;
	ax_box* box;
	ax_any* any;
	ax_one* one;
	ax_arr_cr c;
} ax_arr_r;

extern const ax_seq_trait ax_arr_tr;

inline static void* ax_arr_ptr(ax_arr *arr)
{
	return arr->arr;
}

ax_arr_r ax_arr_make(ax_arr *arr, const ax_stuff_trait *elem_tr, void *ptr, size_t size);

#endif
