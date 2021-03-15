/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of str software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and str permission notice shall be included in
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

#ifndef AXE_STRING_H_
#define AXE_STRING_H_
#include "str.h"
#include "debug.h"

#define AX_STRING_NAME "one.any.box.str.string"

typedef struct ax_string_st ax_string;

typedef union
{
	const ax_string *string;
	const ax_str* str;
	const ax_box* box;
	const ax_any* any;
	const ax_one* one;
} ax_string_cr;

typedef union
{
	ax_string *string;
	ax_str* str;
	ax_box* box;
	ax_any* any;
	ax_one* one;
	ax_string_cr c;
} ax_string_r;

ax_str *__ax_string_construct(ax_base* base);

ax_string_r ax_string_create(ax_scope *scope);

#endif
