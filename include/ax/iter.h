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
#include "stuff.h"

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

#ifndef AX_CITER_TRAIT_DEFINED
#define AX_CITER_TRAIT_DEFINED
typedef struct ax_citer_trait_st ax_citer_trait;
#endif

#ifndef AX_ITER_TRAIT_DEFINED
#define AX_ITER_TRAIT_DEFINED
typedef struct ax_iter_trait_st ax_iter_trait;
#endif

#ifndef AX_BOX_DEFINED
#define AX_BOX_DEFINED
typedef struct ax_box_st ax_box;
#endif

typedef void    *(*ax_iter_get_f)   (const ax_iter *it);
typedef bool  (*ax_iter_comp_f)  (const ax_citer *it1, const ax_citer *it2);
typedef long     (*ax_iter_dist_f)  (const ax_citer *it1, const ax_citer *it2);
typedef void     (*ax_iter_creep_f) (      ax_citer *it);
typedef void     (*ax_iter_move_f)  (      ax_citer *it, long i);
typedef ax_box  *(*ax_iter_box_f)   (const ax_citer *it);

typedef void     (*ax_iter_erase_f)(      ax_iter *it);
typedef ax_fail  (*ax_iter_set_f)  (const ax_iter *it, const void *p);

struct ax_citer_trait_st
{
	ax_iter_move_f  move;
	ax_iter_creep_f prev;
	ax_iter_creep_f next;
	ax_iter_comp_f  less;
	ax_iter_dist_f  dist;
	ax_iter_box_f   box;
	bool         norm;
	unsigned char   type;
};

struct ax_iter_trait_st
{
	const ax_citer_trait ctr; /* Keep this first */
	ax_iter_get_f   get;
	ax_iter_set_f   set;
	ax_iter_erase_f erase;
};

struct ax_citer_st
{
	const void *owner;
	const ax_citer_trait *tr;
	void *point;
};

struct ax_iter_st
{
	void *owner;
	const ax_iter_trait *tr;
	void *point;
};

inline static ax_citer *ax_iter_c(const ax_iter *it)
{
	register const void *p = it;
	return (ax_citer*) p;
}

inline static bool ax_citer_norm (const ax_citer *it)
{
	return it->tr->norm;
}

inline static bool ax_iter_norm (const ax_iter *it)
{
	return ax_citer_norm(ax_iter_c(it));
}

inline static void ax_citer_move(ax_citer *it, long s)
{
	it->tr->move(it, s);
}

inline static void ax_iter_move(ax_iter *it, long s)
{
	ax_citer_move(ax_iter_c(it), s);
}

inline static void ax_citer_next(ax_citer *it)
{
	it->tr->next(it);
}

inline static void ax_iter_next(ax_iter *it)
{
	ax_citer_next(ax_iter_c(it));
}

inline static void ax_citer_prev(ax_citer *it)
{
	it->tr->prev(it);
}

inline static void ax_iter_prev(ax_iter *it)
{
	ax_citer_prev(ax_iter_c(it));
}

inline static void ax_iter_erase(ax_iter *it)
{
	it->tr->erase(it);
}

void *ax_iter_get(const ax_iter *it);

inline static const void *ax_citer_get(const ax_citer *it)
{
	void *p = (void *) it;
	return ax_iter_get(p);
}

inline static ax_fail ax_iter_set(const ax_iter *it, const void *val)
{
	return it->tr->set(it, val);
}

inline static bool ax_citer_equal(const ax_citer *it1, const ax_citer *it2)
{
	return it1->owner == it2->owner && it1->tr == it2->tr && it1->point == it2->point;
}

inline static bool ax_iter_equal(const ax_iter *it1, const ax_iter *it2)
{
	return ax_citer_equal(ax_iter_c(it1), ax_iter_c(it2));
}

inline static bool ax_citer_less(const ax_citer *it1, const ax_citer *it2)
{
	return it1->tr->less(it1, it2);
}

inline static bool ax_iter_less(const ax_iter *it1, const ax_iter *it2)
{
	return ax_citer_less(ax_iter_c(it1), ax_iter_c(it2));
}

inline static long ax_citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	return it1->tr->dist(it1, it2);
}

inline static long ax_iter_dist(const ax_iter *it1, const ax_iter *it2)
{
	return ax_citer_dist(ax_iter_c(it1), ax_iter_c(it2));
}

inline static bool ax_citer_is(const ax_citer *it, int type)
{
	return (it->tr->type & type) == type;
}

inline static bool ax_iter_is(const ax_iter *it, int type)
{
	return ax_citer_is(ax_iter_c(it), type);
}

inline static const ax_box *ax_citer_box(const ax_citer *it)
{
	return it->tr->box(it);
}

inline static ax_box *ax_iter_box(const ax_iter *it)
{
	return it->tr->ctr.box(ax_iter_c(it));
}

void ax_iter_swap(const ax_iter *it1, const ax_iter *it2);

ax_citer ax_citer_npos(const ax_citer *it);

inline static ax_iter ax_iter_npos(const ax_iter *it)
{
	ax_citer citer = ax_citer_npos(ax_iter_c(it));
	void *p = &citer;
	return *(ax_iter *) p;
}

#endif
