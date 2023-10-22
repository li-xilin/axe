/*
 * Copyright (c) 2021-2023 Li Xilin <lixilin@gmx.com>
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

typedef int ax_dump_out_cb_f(const char *str, size_t len, void *ctx);

typedef struct ax_dump_format_st
{
	int (*snumber)(intmax_t value, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*unumber)(uintmax_t value, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*fnumber)(double value, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*pointer)(const void *value, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*string)(const char *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*wstring)(const wchar_t *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*memory)(const ax_byte *value, size_t size, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*symbol)(const char *name, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*pair_left)(ax_dump_out_cb_f *out_cb, void *ctx);
	int (*pair_midst)(ax_dump_out_cb_f *out_cb, void *ctx);
	int (*pair_right)(ax_dump_out_cb_f *out_cb, void *ctx);
	int (*block_left)(const char *label, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*block_midst)(size_t index, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*block_right)(const char *label, ax_dump_out_cb_f *out_cb, void *ctx);
	int (*nomem)(ax_dump_out_cb_f *out_cb, void *ctx);
	int (*indent)(int depth, ax_dump_out_cb_f *out_cb, void *ctx);
} ax_dump_format;

ax_dump *ax_dump_int(intmax_t val);

ax_dump *ax_dump_uint(uintmax_t val);

ax_dump *ax_dump_float(double val);

ax_dump *ax_dump_ptr(const void *val);

ax_dump *ax_dump_str(const char *val);

ax_dump *ax_dump_wcs(const wchar_t *val);

ax_dump *ax_dump_mem(const void *ptr, size_t size);

ax_dump *ax_dump_symbol(const char *sym);

ax_dump *ax_dump_pair(ax_dump *d1, ax_dump *d2);

ax_dump *ax_dump_block(const char* name, size_t elem_cnt);

ax_fail ax_dump_set_name(ax_dump *dmp, const char *sym);

void ax_dump_bind(ax_dump *dmp, int index, ax_dump* binding);

inline static void ax_dump_named_bind(ax_dump *dmp, int index, const char *name, ax_dump* binding)
{
	ax_dump_bind(dmp, index, ax_dump_pair(ax_dump_symbol(name), binding));
}

void ax_dump_free(ax_dump *dmp);

ax_fail ax_dump_fput(const ax_dump *dmp, const ax_dump_format *format, FILE *fp);

int ax_dump_serialize(const ax_dump *dmp, const ax_dump_format *format, ax_dump_out_cb_f *cb, void *ctx);

const ax_dump_format *ax_dump_get_default_format();

#endif
