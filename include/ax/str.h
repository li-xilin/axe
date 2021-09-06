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
#include "def.h"
#include <stdarg.h>

#define AX_STR_NAME AX_SEQ_NAME ".str"

#ifndef AX_STR_DEFINED
#define AX_STR_DEFINED
typedef struct ax_str_st ax_str;
#endif

#ifndef AX_STR_TRAIT_DEFINED
#define AX_STR_TRAIT_DEFINED
typedef struct ax_str_trait_st ax_str_trait;
#endif

typedef ax_fail  (*ax_str_append_f) (      ax_str *str, const char *s);
typedef size_t   (*ax_str_length_f) (const ax_str *str);
typedef ax_fail  (*ax_str_insert_f) (      ax_str *str, size_t start, const char *s);
typedef char    *(*ax_str_strz_f)   (      ax_str *str);
typedef int      (*ax_str_comp_f)   (const ax_str *str, const char *s);
typedef ax_str  *(*ax_str_substr_f) (const ax_str *str, size_t start, size_t len);
typedef ax_seq  *(*ax_str_split_f)  (const ax_str *str, const char ch);
typedef ax_fail  (*ax_str_sprintf_f)(      ax_str *str, const char *fmt, va_list args); 

struct ax_str_trait_st
{
	const ax_seq_trait seq;
	const ax_str_append_f  append;
	const ax_str_length_f  length;
	const ax_str_insert_f  insert;
	const ax_str_strz_f    strz;
	const ax_str_comp_f    comp;
	const ax_str_substr_f  substr;
	const ax_str_split_f   split;
	const ax_str_sprintf_f sprintf;
};

struct ax_str_st
{
	const ax_str_trait* tr;
	ax_seq_env env;
};

typedef union
{
	const ax_str *str;
	const ax_seq *seq;
	const ax_box *box;
	const ax_any *any;
	const ax_one *one;
} ax_str_cr;

typedef union
{
	ax_str *str;
	ax_seq *seq;
	ax_box *box;
	ax_any *any;
	ax_one *one;
} ax_str_r;

inline static ax_fail  ax_str_append (      ax_str* str, const char *s) {return str->tr->append(str, s); }
inline static size_t   ax_str_length (const ax_str* str) { return str->tr->length(str); }
inline static ax_fail  ax_str_insert (      ax_str* str, size_t start, const char *s) { return str->tr->insert(str, start, s); }
inline static char    *ax_str_strz   (      ax_str* str) { return str->tr->strz(str); }
inline static int      ax_str_comp   (const ax_str* str, const char* s) { return str->tr->comp(str, s); }
inline static ax_str  *ax_str_substr (const ax_str* str, size_t start, size_t len) { return str->tr->substr(str, start, len); }
inline static ax_seq  *ax_str_split  (const ax_str* str, const char ch) { return str->tr->split(str, ch); }

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
