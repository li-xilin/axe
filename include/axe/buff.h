/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AXE_BUFF_H_
#define AXE_BUFF_H_

#include "debug.h"
#include "any.h"
#include "def.h"

#ifndef AX_POOL_DEFINED
#define AX_POOL_DEFINED
typedef struct ax_pool_st ax_pool;
#endif

#ifndef AX_BUFF_DEFINED
#define AX_BUFF_DEFINED
typedef struct ax_buff_st ax_buff;
#endif

typedef union
{
	const ax_buff *buff;
	const ax_any *any;
	const ax_one *one;
} ax_buff_cr;

typedef union
{
	ax_buff *buff;
	ax_any *any;
	ax_one *one;
	ax_buff_cr c;
} ax_buff_r;

ax_any *__ax_buff_construct(ax_base *base);

ax_buff_r ax_buff_create(ax_scope *scope);

ax_fail ax_buff_set_max(ax_buff *buff, size_t max);

ax_fail ax_buff_adapt(ax_buff *buff, size_t size);

ax_fail ax_buff_resize(ax_buff *buff, size_t size);

ax_fail ax_buff_alloc(ax_buff *buff, size_t size, void **obuf);

ax_fail ax_buff_shrink(ax_buff *buff);

ax_fail ax_buff_reserve(ax_buff *buff, size_t size);

size_t ax_buff_size(const ax_buff *buff, size_t *real);

size_t ax_buff_max(const ax_buff *buff);

size_t ax_buff_min(const ax_buff *buff);

ax_byte *ax_buff_ptr(ax_buff *buff);

const ax_byte *ax_buff_const_ptr(const ax_buff *buff);

#endif
