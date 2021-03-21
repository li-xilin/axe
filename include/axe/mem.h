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

#ifndef AXE_MEM_H_
#define AXE_MEM_H_

#include "def.h"

#ifndef AX_POOL_DEFINED
#define AX_POOL_DEFINED
typedef struct ax_pool_st ax_pool;
#endif

#define ax_mem_pswap(_a, _b, _type) \
do { \
	_type tmp = *(_type*)(_a); \
	*(_type*) (_a) = *(_type*) (_b); \
	*(_type*) (_b) = tmp; \
} while(0)

void ax_memxor(void *ptr1, void *ptr2, size_t size);

char *ax_strdup(ax_pool *pool, const char *str);

wchar_t *ax_wcsdup(ax_pool *pool, const wchar_t* str);

size_t ax_strhash(const char *s);

size_t ax_wcshash(const wchar_t *s);

size_t ax_memhash(const unsigned char *p, size_t size);

#endif

