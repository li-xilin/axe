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
#include "debug.h"
#include "seq.h"
#include <stdarg.h>

#define AX_STR_NAME AX_SEQ_NAME ".str"

#ifndef AX_STR_DEFINED
#define AX_STR_DEFINED
typedef struct ax_str_st ax_str;
#endif

#define AX_CLASS_BASE_str seq
#define AX_CLASS_ROLE_str(_l) _l AX_CLASS_PTR(str); AX_CLASS_ROLE_seq(_l)

AX_BEGIN_TRAIT(str)
	ax_fail  (*append) (      ax_str *str, const char *s);
	size_t   (*length) (const ax_str *str);
	ax_fail  (*insert) (      ax_str *str, size_t start, const char *s);
	char    *(*strz)   (      ax_str *str);
	int      (*comp)   (const ax_str *str, const char *s);
	ax_str  *(*substr) (const ax_str *str, size_t start, size_t len);
	ax_seq  *(*split)  (const ax_str *str, const char ch);
	ax_fail  (*sprintf)(      ax_str *str, const char *fmt, va_list args); 
AX_END;
AX_BEGIN_ENV(str) AX_END;
AX_BLESS(str);

inline static ax_fail ax_str_append(ax_str* str, const char *s) {
	ax_require(str, str->tr->append);
	return str->tr->append(str, s);
}

inline static size_t ax_str_length(const ax_str* str)
{
	ax_require(str, str->tr->length);
	return str->tr->length(str);
}

inline static ax_fail ax_str_insert(ax_str* str, size_t start, const char *s)
{
	ax_require(str, str->tr->insert);
	return str->tr->insert(str, start, s);
}

inline static char *ax_str_strz(ax_str* str)
{
	ax_require(str, str->tr->strz);
	return str->tr->strz(str);
}

inline static int ax_str_comp(const ax_str* str, const char* s)
{
	ax_require(str, str->tr->comp);
	return str->tr->comp(str, s);
}

inline static ax_str *ax_str_substr(const ax_str* str, size_t start, size_t len)
{
	ax_require(str, str->tr->substr);
	return str->tr->substr(str, start, len);
}

inline static ax_seq  *ax_str_split(const ax_str* str, const char ch)
{
	ax_require(str, str->tr->split);
	return str->tr->split(str, ch);
}

inline static ax_fail ax_str_sprintf(ax_str* str, const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	ax_fail fail = str->tr->sprintf(str, fmt, args);
	va_end(args);
	return fail;
}

inline static const char *ax_str_cstrz(const ax_str* str)
{
	return ax_str_strz((ax_str *)str);
}

#endif
