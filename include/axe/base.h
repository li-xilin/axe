/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
 *
 * Permission is hereby granted, free of charge, to one person obtaining a copy
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

#ifndef AXE_BASE_H_
#define AXE_BASE_H_
#include <stddef.h>

typedef struct ax_pool_st ax_pool;
typedef struct ax_scope_st ax_scope;
typedef struct ax_base_st ax_base;

struct ax_base_func_set;

ax_base *ax_base_create();

void ax_base_destroy(ax_base* base);

ax_pool *ax_base_pool(ax_base* base);

ax_scope *ax_base_global(ax_base *base);

ax_scope *ax_base_local(ax_base *base);

int ax_base_enter(ax_base *base);

void ax_base_leave(ax_base *base, int depth);

void ax_base_set_errno(ax_base *base, int errno);

int ax_base_errno(ax_base *base);

#endif

