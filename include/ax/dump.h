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

#ifndef AX_DUMP_H
#define AX_DUMP_H

#include "def.h"
#include "stdio.h"

#ifndef AX_DUMP_DEFINED
#define AX_DUMP_DEFINED
typedef struct ax_dump_st ax_dump;
#endif

typedef int (ax_dump_out_cb)(const char *str, size_t len, void *ctx);

ax_dump *ax_dump_int(int64_t val);

ax_dump *ax_dump_uint(uint64_t val);

ax_dump *ax_dump_float(double val);

ax_dump *ax_dump_ptr(const void *val);

ax_dump *ax_dump_str(const char *val);

ax_dump *ax_dump_wcs(const wchar_t *val);

ax_dump *ax_dump_mem(const void *ptr, size_t size);

ax_dump *ax_dump_pair();

ax_dump *ax_dump_block(const char* name, size_t elem_cnt);

void ax_dump_bind(ax_dump *dmp, int index, ax_dump* binding);

void ax_dump_free(ax_dump *dmp);

ax_fail ax_dump_fput(const ax_dump *dmp, FILE *fp);

int ax_dump_out(const ax_dump *dmp, ax_dump_out_cb *cb, void *ctx);

#endif
