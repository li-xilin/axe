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

#ifndef AX_SCOPE_H
#define AX_SCOPE_H
#include "one.h"
#include "debug.h"

#define AX_SCOPE_NAME AX_ONE_NAME ".scope"

typedef union
{
	const ax_one *one;
	const ax_scope *scope;
} ax_scope_cr;

typedef union
{
	ax_one *one;
	ax_scope *scope;
	ax_scope_cr c;
} ax_scope_r;

ax_one *__ax_scope_construct(ax_base *base);

ax_scope_r ax_scope_create(ax_scope *scope);

void ax_scope_attach(ax_scope *scope, ax_one *one);

void ax_scope_clean(ax_scope *scope);

void ax_scope_destroy(ax_scope *scope);

ax_bool ax_scope_detach(ax_one *one);

#endif