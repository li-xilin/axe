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

#ifndef AX_STR_H
#define AX_STR_H
#include "seq.h"
#include <stdarg.h>

#ifndef AX_STR_DEFINED
#define AX_STR_DEFINED
typedef struct ax_str_st ax_str;
#endif

#define ax_baseof_ax_str ax_seq

ax_abstract_code_begin(ax_str)
	ax_fail  (*append) (      ax_str *str, const char *s);
	size_t   (*length) (const ax_str *str);
	ax_fail  (*insert) (      ax_str *str, size_t start, const char *s);
	char    *(*strz)   (      ax_str *str);
	int      (*comp)   (const ax_str *str, const char *s);
	ax_str  *(*substr) (const ax_str *str, size_t start, size_t len);
	ax_seq  *(*split)  (const ax_str *str, const char ch);
	ax_fail  (*sprintf)(      ax_str *str, const char *fmt, va_list args); 
ax_end;

ax_abstract_data_begin(ax_str) ax_end;

ax_abstract_declare(4, ax_str);

inline static ax_fail ax_str_append(ax_str* str, const char *s) {
	return ax_obj_do(str, append, s);
}

inline static size_t ax_str_length(const ax_str* str)
{
	return ax_box_size(ax_cr(ax_str, str).ax_box);
}

inline static ax_fail ax_str_insert(ax_str* str, size_t start, const char *s)
{
	return ax_obj_do(str, insert, start, s);
}

inline static char *ax_str_strz(ax_str* str)
{
	return ax_obj_do0(str, strz);
}

inline static int ax_str_comp(const ax_str* str, const char* s)
{
	return ax_obj_do(str, comp, s);
}

inline static ax_str *ax_str_substr(const ax_str* str, size_t start, size_t len)
{
	return ax_obj_do(str, substr, start, len);
}

inline static ax_seq_r ax_str_split(const ax_str* str, const char ch)
{
	return ax_r(ax_seq, ax_obj_do(str, split, ch));
}

inline static ax_fail ax_str_sprintf(ax_str* str, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ax_fail fail = ax_obj_do(str, sprintf, fmt, args);
	va_end(args);
	return fail;
}

inline static const char *ax_str_cstrz(const ax_str* str)
{
	return ax_str_strz((ax_str *)str);
}

#endif
