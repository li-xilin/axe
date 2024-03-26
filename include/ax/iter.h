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

#ifndef AX_ITER_H
#define AX_ITER_H
#include "debug.h"
#include "trait.h"
#include "mem.h"
#include <stdint.h>

#define AX_IT_IN   ((1 << 0))
#define AX_IT_FORW ((1 << 1) | AX_IT_IN)
#define AX_IT_BID  ((1 << 2) | AX_IT_IN | AX_IT_FORW)
#define AX_IT_RAND ((1 << 3) | AX_IT_IN | AX_IT_FORW | AX_IT_BID)
#define AX_IT_OUT  ((1 << 4))

#ifndef AX_ITER_DEFINED
#define AX_ITER_DEFINED
typedef struct ax_iter_st ax_iter;
#endif

#ifndef AX_CITER_DEFINED
#define AX_CITER_DEFINED
typedef struct ax_citer_st ax_citer;
#endif

#ifndef AX_BOX_DEFINED
#define AX_BOX_DEFINED
typedef struct ax_box_st ax_box;
#endif

typedef struct ax_iter_trait_st
{
	bool     (*equal) (const ax_citer *it1, const ax_citer *it2);
	bool     (*less)  (const ax_citer *it1, const ax_citer *it2);
	long     (*dist)  (const ax_citer *it1, const ax_citer *it2);
	void     (*prev)  (      ax_citer *it);
	void     (*next)  (      ax_citer *it);
	void     (*move)  (      ax_citer *it, long i);
	void     (*erase) (      ax_iter *it);
	ax_fail  (*set)   (const ax_iter *it, const void *p, va_list *ap);
	void     (*swap)  (ax_iter *it1, ax_iter *it2);
	void    *(*get)   (const ax_citer *it);
	ax_box  *(*box)   (const ax_citer *it);
	bool     (norm);
	unsigned char (type);
} ax_iter_trait;

#define __AX_ITER_STRUCT_BLOCK(_const) \
{ \
	_const void *owner; \
	const struct ax_iter_trait_st *tr; \
	const struct ax_trait_st *etr; \
	void *point; \
	uintptr_t extra; \
}
struct ax_iter_st __AX_ITER_STRUCT_BLOCK();
struct ax_citer_st __AX_ITER_STRUCT_BLOCK(const);
#undef __AX_ITER_STRUCT_BLOCK

inline static ax_citer *ax_iter_c(ax_iter *it)
{
	void *p = it;
	return (ax_citer *)p;
}

inline static const ax_citer *ax_iter_cc(const ax_iter *it)
{
	const void *p = it;
	return (ax_citer *)p;
}

inline static ax_citer ax_iter_citer(ax_iter it)
{
	return *ax_iter_c(ax_p(ax_iter, it));
}

inline static bool ax_citer_norm (const ax_citer *it)
{
	return it->tr->norm;
}

inline static bool ax_iter_norm (const ax_iter *it)
{
	return ax_citer_norm(ax_iter_cc(it));
}

inline static void ax_citer_move(ax_citer *it, long s)
{
	ax_assert(it->tr->move, "operation not supported");
	it->tr->move(it, s);
}

inline static void ax_iter_move(ax_iter *it, long s)
{
	ax_citer_move(ax_iter_c(it), s);
}

inline static void ax_citer_next(ax_citer *it)
{
	ax_assert(it->tr->next, "operation not supported");
	it->tr->next(it);
}

inline static void ax_iter_next(ax_iter *it)
{
	ax_citer_next(ax_iter_c(it));
}

inline static void ax_citer_prev(ax_citer *it)
{
	ax_assert(it->tr->prev, "operation not supported");
	it->tr->prev(it);
}

inline static void ax_iter_prev(ax_iter *it)
{
	ax_citer_prev(ax_iter_c(it));
}

inline static ax_citer ax_citer_ret_next(const ax_citer *it)
{
	ax_assert(it->tr->next, "operation not supported");
	ax_citer next = *it;
	it->tr->next(&next);
	return next;
}

inline static ax_iter ax_iter_ret_next(const ax_iter *it)
{
	ax_assert(it->tr->next, "operation not supported");
	ax_iter next = *it;
	it->tr->next(ax_iter_c(&next));
	return next;
}

inline static ax_citer ax_citer_ret_prev(const ax_citer *it)
{
	ax_assert(it->tr->prev, "operation not supported");
	ax_citer prev = *it;
	it->tr->prev(&prev);
	return prev;
}

inline static ax_iter ax_iter_ret_prev(const ax_iter *it)
{
	ax_assert(it->tr->prev, "operation not supported");
	ax_iter prev = *it;
	it->tr->prev(ax_iter_c(&prev));
	return prev;
}

inline static void ax_iter_erase(ax_iter *it)
{
	ax_assert(it->tr->erase, "operation not supported");
	it->tr->erase(it);
}

inline static void *ax_iter_get(const ax_iter *it)
{
	ax_assert(it->tr->get, "operation not supported");
	return ax_trait_out(it->etr, it->tr->get(ax_iter_cc(it)));
}

inline static const void *ax_citer_get(const ax_citer *it)
{
	ax_assert(it->tr->get, "operation not supported");
	return ax_trait_out(it->etr, it->tr->get(it));
}

inline static ax_fail ax_iter_set(const ax_iter *it, const void *val)
{
	ax_assert(it->tr->set, "operation not supported");
	return it->tr->set(it, ax_trait_in(it->etr, val), NULL);
}

inline static ax_fail ax_iter_iset(const ax_iter *it, ...)
{
	ax_assert(it->tr->set, "operation not supported");
	va_list ap;
	va_start(ap, it);
	ax_fail fail = it->tr->set(it, NULL, &ap);
	va_end(ap);
	return fail;
}

inline static bool ax_citer_equal(const ax_citer *it1, const ax_citer *it2)
{
	return (uintptr_t)(it1->owner == it2->owner)
		& (uintptr_t)(it1->tr == it2->tr)
		& (uintptr_t)(it1->etr == it2->etr)
		& (uintptr_t)(it1->point == it2->point);
}

inline static bool ax_iter_equal(const ax_iter *it1, const ax_iter *it2)
{
	return ax_citer_equal(ax_iter_cc(it1), ax_iter_cc(it2));
}

inline static bool ax_citer_less(const ax_citer *it1, const ax_citer *it2)
{
	return it1->tr->less(it1, it2);
}

inline static bool ax_iter_less(const ax_iter *it1, const ax_iter *it2)
{
	return ax_citer_less(ax_iter_cc(it1), ax_iter_cc(it2));
}

inline static long ax_citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	ax_assert(it1->tr->dist, "operation not supported");
	return it1->tr->dist(it1, it2);
}

inline static long ax_iter_dist(const ax_iter *it1, const ax_iter *it2)
{
	return ax_citer_dist(ax_iter_cc(it1), ax_iter_cc(it2));
}

inline static bool ax_citer_is(const ax_citer *it, int type)
{
	return (it->tr->type & type) == type;
}

inline static bool ax_iter_is(const ax_iter *it, int type)
{
	return ax_citer_is(ax_iter_cc(it), type);
}

inline static const ax_box *ax_citer_box(const ax_citer *it)
{
	ax_assert(it->tr->box, "operation not supported");
	return it->tr->box(it);
}

inline static ax_box *ax_iter_box(const ax_iter *it)
{
	ax_assert(it->tr->box, "operation not supported");
	return it->tr->box(ax_iter_cc(it));
}

inline static void ax_iter_swap(ax_iter *it1, ax_iter *it2)
{
	if (it1->tr->swap) {
		it1->tr->swap(it1, it2);
	}
	else {
		ax_assert(it1->tr->get && it2->tr->get, "operation not supported");
		ax_memswp(it1->tr->get(ax_iter_cc(it1)),
				it2->tr->get(ax_iter_cc(it2)), ax_trait_size(it1->etr));
	}
}

const char *ax_iter_type_str(int type);

#endif
