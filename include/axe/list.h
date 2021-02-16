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

#ifndef AXE_LIST_H_
#define AXE_LIST_H_
#include "seq.h"

#define AX_LIST_NAME "one.any.box.seq.list"

typedef struct ax_list_st ax_list;

typedef union
{
	ax_one *one;
	ax_any *any;
	ax_box *box;
	ax_seq *seq;
	ax_list *list;
} ax_list_role;

ax_seq *__ax_list_construct(ax_base *base, const ax_stuff_trait *elem_tr);

ax_list_role ax_list_create(ax_scope *scope, const ax_stuff_trait *elem_tr);

ax_list_role ax_list_init(ax_scope *scope, const char *fmt, ...);


#endif
