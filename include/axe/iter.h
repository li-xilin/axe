/*
  *Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
 *
  *Permission is hereby granted, free of charge, to any person obtaining a copy
  *of this software and associated documentation files (the "Software"), to deal
  *in the Software without restriction, including without limitation the rights
  *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  *copies of the Software, and to permit persons to whom the Software is
  *furnished to do so, subject to the following conditions:
 *
  *The above copyright notice and this permission notice shall be included in
  *all copies or substantial portions of the Software.
 *
  *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  *THE SOFTWARE.
 */

#ifndef ITER_H_
#define ITER_H_
#include "debug.h"
#include "stuff.h"

#include <stdint.h>

#define AX_IT_IN   0x01
#define AX_IT_FORW 0x03
#define AX_IT_BID  0x05
#define AX_IT_RAND 0x07
#define AX_IT_OUT  0x08

typedef struct ax_iter_st ax_iter;
typedef struct ax_iter_trait_st ax_iter_trait;
typedef struct ax_box_st ax_box;

typedef void    *(*ax_iter_get_f)  (const ax_iter *it);
typedef ax_fail  (*ax_iter_set_f)  (const ax_iter *it, const void *p);
typedef ax_bool  (*ax_iter_comp_f) (const ax_iter *it1, const ax_iter *it2);
typedef long     (*ax_iter_dist_f) (const ax_iter *it1, const ax_iter *it2);
typedef void     (*ax_iter_erase_f)(ax_iter *it);
typedef void     (*ax_iter_creep_f)(ax_iter *it);
typedef void     (*ax_iter_move_f) (ax_iter *it, long i);

struct ax_iter_trait_st
{
	ax_iter_move_f  move;
	ax_iter_creep_f prev;
	ax_iter_creep_f next;
	ax_iter_get_f   get;
	ax_iter_set_f   set;
	ax_iter_comp_f  equal;
	ax_iter_comp_f  less;
	ax_iter_dist_f  dist;
	ax_iter_erase_f erase;
	ax_bool         norm;
	unsigned char   type;
};

struct ax_iter_st
{
	void *owner;
	const ax_iter_trait *tr;
	void *point;
};

inline static ax_bool ax_iter_norm (ax_iter it)
{
	return it.tr->norm;
}

inline static ax_iter ax_iter_move(ax_iter it, long s) {
	it.tr->move((&it), s);
	return it;
}

inline static ax_iter ax_iter_next(ax_iter it)
{
	it.tr->next(&it);
	return it;

}

inline static ax_iter ax_iter_prev(ax_iter it)
{
	it.tr->prev(&it);
	return it;
}

inline static ax_iter ax_iter_erase(ax_iter it)
{
	it.tr->erase(&it);
	return it;
}
inline static void *ax_iter_get(ax_iter it)
{
	return it.tr->get((&it));
}

inline static ax_fail ax_iter_set(ax_iter it, const void *val)
{
	return it.tr->set(&it, val);
}

inline static ax_bool ax_iter_equal(ax_iter it1, ax_iter it2)
{
	ax_assert(it1.owner == it2.owner, "different owner of two iterator");
	ax_assert(it1.tr == it2.tr, "different trait of two iterator");
	return it1.point == it2.point;
	//return it1.tr->equal(&it1, &it2);
}

inline static ax_bool ax_iter_less(ax_iter it1, const ax_iter it2)
{
	return it1.tr->equal(&it1, &it2);
}

inline static long ax_iter_dist(ax_iter it1, const ax_iter it2)
{
	return it1.tr->dist(&it1, &it2);
}

inline static ax_bool ax_iter_is(const ax_iter *it, int type)
{
	return (it->tr->type & type) == type;
}

void ax_iter_swap(ax_iter it1, ax_iter it2);

ax_iter ax_iter_npos(void *owner, const ax_iter_trait *tr);

#endif
