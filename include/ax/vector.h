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

#ifndef AX_VECTOR_H
#define AX_VECTOR_H
#include "seq.h"

#define AX_VECTOR_NAME AX_SEQ_NAME ".vector"

typedef struct ax_vector_st ax_vector;

typedef union
{
	const ax_vector* vector;
	const ax_seq* seq;
	const ax_box* box;
	const ax_any* any;
	const ax_one* one;
} ax_vector_cr;

typedef union
{
	ax_vector* vector;
	ax_seq* seq;
	ax_box* box;
	ax_any* any;
	ax_one* one;
	ax_vector_cr c;
} ax_vector_r;

extern const ax_seq_trait ax_vector_tr;

ax_seq*__ax_vector_construct(const ax_stuff_trait* elem_tr);

ax_vector_r ax_vector_create(ax_scope* scope, const ax_stuff_trait* elem_tr);

void* ax_vector_buffer(ax_vector *vecor);

inline static ax_vector_r ax_vector_init(ax_scope *scope, const char *fmt, ...)
{
	va_list varg;
	va_start(varg, fmt);
	ax_vector_r role = { .seq = ax_seq_vinit(scope, __ax_vector_construct, fmt, varg) };
	va_end(varg);
	return role;
}

#endif
