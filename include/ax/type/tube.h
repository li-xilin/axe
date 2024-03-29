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

#ifndef AX_TUBE_H
#define AX_TUBE_H
#include "box.h"

#ifndef AX_TUBE_DEFINED
#define AX_TUBE_DEFINED
typedef struct ax_tube_st ax_tube;
#endif

typedef ax_tube *(ax_tube_construct_f)(const ax_trait *elem_tr);

#define ax_baseof_ax_tube ax_any

ax_abstract_code_begin(ax_tube)
	ax_fail (*push)   (ax_tube *tube, const void *val, va_list *ap);
	void    (*pop)    (ax_tube *tube);
	const void *(*prime)  (const ax_tube *tube);
	size_t  (*size)   (const ax_tube *tube);
ax_end;

ax_abstract_data_begin(ax_tube)
	const ax_trait *const elem_tr;
ax_end;

ax_abstract_declare(2, ax_tube);

inline static ax_fail ax_tube_push(ax_tube *tube, const void *val)
{
	return ax_obj_do(tube, push, val, NULL);
}

inline static ax_fail ax_tube_ipush(ax_tube *tube, ...)
{
	va_list ap;
	va_start(ap, tube);
	ax_fail fail = ax_obj_do(tube, push, NULL, &ap);
	va_end(ap);
	return fail;
}

inline static void ax_tube_pop(ax_tube *tube)
{
	ax_obj_do0(tube, pop);
}

static inline const void *ax_tube_prime(ax_tube *tube)
{
	return ax_obj_do0(tube, prime);
}

static inline const void *ax_tube_cprime(const ax_tube *tube)
{
	return ax_obj_do0(tube, prime);
}

static inline size_t ax_tube_size(const ax_tube *tube)
{
	return ax_obj_do0(tube, size);
}

#endif
