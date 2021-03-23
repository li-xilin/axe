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

#ifndef AXE_BOX_H_
#define AXE_BOX_H_
#include "iter.h"
#include "any.h"

#ifndef AX_BOX_TRAIT_DEFINED
#define AX_BOX_TRAIT_DEFINED
typedef struct ax_box_trait_st ax_box_trait;
#endif

#define ax_foreach(_type, _var, _box)                                                   \
	for ( int __ax_foreach_##_var##_flag = 1 ; __ax_foreach_##_var##_flag ; )       \
	for (_type  _var ; __ax_foreach_##_var##_flag ; __ax_foreach_##_var##_flag = 0) \
	for ( ax_citer __##_var##_iter = ax_box_cbegin(_box),                           \
			__##_var##_end_iter = ax_box_cend(_box);                        \
		!ax_citer_equal(&__##_var##_iter, &__##_var##_end_iter)                 \
			&& ((_var) = ax_citer_get(&__##_var##_iter), 1);                  \
		ax_citer_next(&__##_var##_iter))

typedef size_t  (*ax_box_size_f)  (const ax_box* box);
typedef ax_iter (*ax_box_iter_f)  (      ax_box* box);
typedef void    (*ax_box_clear_f) (      ax_box* box);
typedef const ax_stuff_trait *(*ax_box_elem_tr_f) (const ax_box* box);

struct ax_box_trait_st
{
	const ax_box_size_f   size;
	const ax_box_size_f   maxsize;

	const ax_box_iter_f   begin;
	const ax_box_iter_f   end;
	const ax_box_iter_f   rbegin;
	const ax_box_iter_f   rend;

	const ax_box_clear_f  clear;
	const ax_box_elem_tr_f elem_tr;
};

struct ax_box_st
{
	ax_any _any;
	const ax_box_trait *const tr;
};

typedef union
{
	const ax_box *box;
	const ax_any *any;
	const ax_one *one;
} ax_box_cr;

typedef union
{
	ax_box *box;
	ax_any *any;
	ax_one *one;
	ax_box_cr c;
} ax_box_r;

static inline size_t ax_box_size(const ax_box* box)
{
	ax_trait_require(box, box->tr->size);
	return box->tr->size(box);
}

static inline size_t ax_box_maxsize(const ax_box* box)
{
	ax_trait_require(box, box->tr->maxsize);
	return box->tr->maxsize(box);
}

static inline ax_iter ax_box_begin(ax_box* box)
{
	ax_trait_require(box, box->tr->begin);
	return box->tr->begin(box);
}

static inline ax_iter ax_box_end(ax_box* box)
{
	ax_trait_require(box, box->tr->end);
	return box->tr->end(box);
}

static inline ax_iter ax_box_rbegin(ax_box* box)
{
	ax_trait_optional(box, box->tr->rbegin);
	return box->tr->rbegin(box);
}

static inline ax_iter ax_box_rend(ax_box* box)
{
	ax_trait_optional(box, box->tr->rend);
	return box->tr->rend(box);
}

static inline ax_citer ax_box_cbegin(const ax_box* box)
{
	ax_trait_require(box, box->tr->begin);
	ax_iter iter = ax_box_begin((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline ax_citer ax_box_cend(const ax_box* box)
{
	ax_trait_require(box, box->tr->end);
	ax_iter iter = ax_box_end((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline ax_citer ax_box_crbegin(const ax_box* box)
{
	ax_trait_optional(box, box->tr->rbegin);
	ax_iter iter = box->tr->rbegin((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline ax_citer ax_box_crend(const ax_box* box)
{
	ax_trait_optional(box, box->tr->rend);
	ax_iter iter = box->tr->rend((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline void ax_box_clear(ax_box* box)
{
	ax_trait_require(box, box->tr->clear);
	box->tr->clear(box);
}

static inline const ax_stuff_trait *ax_box_elem_tr(const ax_box* box)
{
	ax_trait_require(box, box->tr->elem_tr);
	return box->tr->elem_tr(box);
}

#endif
