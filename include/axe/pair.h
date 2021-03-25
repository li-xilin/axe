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

#ifndef AXE_PAIR_H_
#define AXE_PAIR_H_
#include "one.h"

#ifndef AX_PAIR_DEFINED
#define AX_PAIR_DEFINED
typedef struct ax_pair_st ax_pair;
#endif

typedef union
{
	const ax_pair *pair;
	const ax_one *one;
} ax_pair_cr;

typedef union
{
	ax_pair *pair;
	ax_one *one;
	ax_pair_cr c;
} ax_pair_r;

ax_pair *__ax_pair_construct(ax_base *base, const void *key, void *value);

ax_pair_r ax_pair_create(ax_scope *scope, const void *key, void *value);

const void *ax_pair_key(const ax_pair *pair);

void *ax_pair_value(ax_pair *pair);

inline static const void *ax_pair_cvalue(const ax_pair *pair)
{
	return ax_pair_value((ax_pair *)pair);
}

#endif
