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

#ifndef AX_SEQ_H
#define AX_SEQ_H
#include "box.h"

#ifndef AX_SEQ_DEFINED
#define AX_SEQ_DEFINED
typedef struct ax_seq_st ax_seq;
#endif

typedef ax_fail (*ax_seq_push_f)   (ax_seq *seq, const void *val, va_list *ap);
typedef ax_fail (*ax_seq_pop_f)    (ax_seq *seq);
typedef void    (*ax_seq_invert_f) (ax_seq *seq);
typedef ax_fail (*ax_seq_trunc_f)  (ax_seq *seq, size_t size);
typedef ax_iter (*ax_seq_at_f)     (const ax_seq *seq, size_t index);
typedef ax_fail (*ax_seq_insert_f) (ax_seq *seq, ax_iter *iter, const void *val, va_list *ap);
typedef void   *(*ax_seq_end_f)    (const ax_seq *seq);

typedef ax_seq *(ax_seq_construct_f)(const ax_trait *tr);

#define ax_baseof_seq box

ax_abstract_env_begin(seq)
ax_end;

ax_abstract_begin(seq)
	const ax_seq_push_f   push;
	const ax_seq_pop_f    pop;
	const ax_seq_push_f   pushf;
	const ax_seq_pop_f    popf;
	const ax_seq_invert_f invert;
	const ax_seq_trunc_f  trunc;
	const ax_seq_at_f     at;
	const ax_seq_insert_f insert;
	const ax_seq_end_f   first;
	const ax_seq_end_f   last;
ax_end;

ax_abstract(3, seq);

inline static ax_fail ax_seq_push(ax_seq *seq, const void *val)
{
	return ax_obj_do(seq, push, ax_trait_in(ax_class_env(ax_r(seq, seq).box).elem_tr, val), NULL);
}

inline static ax_fail ax_seq_ipush(ax_seq *seq, ...)
{
	va_list ap;
	va_start(ap, seq);
	ax_fail fail = ax_obj_do(seq, push, NULL, &ap);
	va_end(ap);
	return fail;
}

inline static ax_fail ax_seq_pop(ax_seq *seq)
{
	return ax_obj_do0(seq, pop);
}

inline static ax_fail ax_seq_pushf(ax_seq *seq, const void *val)
{
	return ax_obj_do(seq, pushf, ax_trait_in(ax_class_env(ax_r(seq, seq).box).elem_tr, val), NULL);
}

inline static ax_fail ax_seq_ipushf(ax_seq *seq, ...)
{
	va_list ap;
	va_start(ap, seq);
	ax_fail fail = ax_obj_do(seq, pushf, NULL, &ap);
	va_end(ap);
	return fail;
}

inline static ax_fail ax_seq_popf(ax_seq *seq)
{
	return ax_obj_do0(seq, popf);
}

inline static void ax_seq_invert(ax_seq *seq)
{
	ax_obj_do0(seq, invert);
}

inline static ax_fail ax_seq_trunc(ax_seq *seq, size_t size)
{
	return ax_obj_do(seq, trunc, size);
}

inline static ax_iter ax_seq_at(ax_seq *seq, size_t index)
{
	return ax_obj_do(seq, at, index);
}

inline static ax_citer ax_seq_cat(const ax_seq *seq, size_t index)
{
	return *ax_iter_c(ax_p(ax_iter, ax_obj_do((ax_seq *)seq, at, index)));
}

static inline ax_fail ax_seq_insert(ax_seq *seq, ax_iter *it, const void *val)
{
	return ax_obj_do(seq, insert, it, ax_trait_in(ax_class_env(ax_r(seq, seq).box).elem_tr, val), NULL);
}

static inline ax_fail ax_seq_iinsert(ax_seq *seq, ax_iter *it, ...)
{
	va_list ap;
	va_start(ap, it);
	ax_fail fail = ax_obj_do(seq, insert, it, NULL, &ap);
	va_end(ap);
	return fail;
}

static inline void *ax_seq_first(ax_seq *seq)
{
	return ax_obj_do0(seq, first);
}

static inline const void *ax_seq_cfirst(const ax_seq *seq)
{
	return ax_obj_do0(seq, first);
}

static inline void *ax_seq_last(ax_seq *seq)
{
	return ax_obj_do0(seq, last);
}

static inline const void *ax_seq_clast(const ax_seq *seq)
{
	return ax_obj_do0(seq, last);
}

size_t ax_seq_array(ax_seq *seq, void *elems[], size_t len);

ax_fail ax_seq_push_arraya(ax_seq *seq, const void *arrp);

ax_dump *ax_seq_dump(const ax_seq *seq);

#endif
