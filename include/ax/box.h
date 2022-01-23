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

#ifndef AX_BOX_H
#define AX_BOX_H
#include "iter.h"
#include "any.h"

#define AX_BOX_NAME AX_ANY_NAME ".box"

#define _ax_box_iterate(_box, _it, _cond)                  \
	for (ax_iter _it = ax_box_begin(_box),             \
			_it_end = ax_box_end(_box);        \
		!ax_iter_equal(&_it, &_it_end) && (_cond); \
		ax_iter_next(&_it))

#define _ax_box_citerate(_box, _it, _cond)                  \
	for (ax_citer _it = ax_box_cbegin(_box),            \
			_it_end = ax_box_cend(_box);        \
		!ax_citer_equal(&_it, &_it_end) && (_cond); \
		ax_citer_next(&_it))

#define ax_box_iterate(_box, _it) _ax_box_iterate(_box, _it, true)

#define ax_box_citerate(_box, _it) _ax_box_citerate(_box, _it, true)

#define ax_box_foreach(_box, _type, _var)                                               \
	for ( int __ax_foreach_##_var##_flag = 1 ; __ax_foreach_##_var##_flag ; )       \
	for (_type  _var = NULL; __ax_foreach_##_var##_flag ; __ax_foreach_##_var##_flag = 0) \
	for ( ax_iter __##_var##_iter = ax_box_begin(_box),                             \
			__##_var##_end_iter = ax_box_end(_box);                         \
		!ax_iter_equal(&__##_var##_iter, &__##_var##_end_iter)                  \
			&& ((_var) = ax_iter_get(&__##_var##_iter), 1);                 \
		ax_iter_next(&__##_var##_iter))

#define ax_box_cforeach(_box, _type, _var)                                              \
	for ( int __ax_foreach_##_var##_flag = 1 ; __ax_foreach_##_var##_flag ; )       \
	for (_type  _var = NULL; __ax_foreach_##_var##_flag ; __ax_foreach_##_var##_flag = 0) \
	for ( ax_citer __##_var##_iter = ax_box_cbegin(_box),                           \
			__##_var##_end_iter = ax_box_cend(_box);                        \
		!ax_citer_equal(&__##_var##_iter, &__##_var##_end_iter)                 \
			&& ((_var) = ax_citer_get(&__##_var##_iter), 1);                \
		ax_citer_next(&__##_var##_iter))

typedef size_t  (*ax_box_size_f)  (const ax_box* box);
typedef ax_iter (*ax_box_iter_f)  (      ax_box* box);
typedef void    (*ax_box_clear_f) (      ax_box* box);

#define AX_CLASS_BASE_box any
#define AX_CLASS_ROLE_box(_l) _l AX_CLASS_PTR(box); AX_CLASS_ROLE_any(_l)

AX_BEGIN_TRAIT(box)
	const ax_iter_trait iter;
	const ax_iter_trait riter;

	const ax_box_size_f size;
	const ax_box_size_f maxsize;

	const ax_box_iter_f begin;
	const ax_box_iter_f end;
	const ax_box_iter_f rbegin;
	const ax_box_iter_f rend;

	const ax_box_clear_f clear;
AX_END;

AX_BEGIN_ENV(box)
	const ax_trait *const elem_tr;
AX_END;

AX_BLESS(box);

static inline size_t ax_box_size(const ax_box* box)
{
	ax_require(box, box->tr->size);
	return box->tr->size(box);
}

static inline size_t ax_box_maxsize(const ax_box* box)
{
	ax_require(box, box->tr->maxsize);
	return box->tr->maxsize(box);
}

static inline ax_iter ax_box_begin(ax_box* box)
{
	ax_require(box, box->tr->begin);
	return box->tr->begin(box);
}

static inline ax_iter ax_box_end(ax_box* box)
{
	ax_require(box, box->tr->end);
	return box->tr->end(box);
}

static inline ax_iter ax_box_rbegin(ax_box* box)
{
	ax_require(box, box->tr->rbegin);
	return box->tr->rbegin(box);
}

static inline ax_iter ax_box_rend(ax_box* box)
{
	ax_require(box, box->tr->rend);
	return box->tr->rend(box);
}

static inline ax_citer ax_box_cbegin(const ax_box* box)
{
	ax_require(box, box->tr->begin);
	ax_iter iter = ax_box_begin((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline ax_citer ax_box_cend(const ax_box* box)
{
	ax_require(box, box->tr->end);
	ax_iter iter = ax_box_end((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline ax_citer ax_box_crbegin(const ax_box* box)
{
	ax_require(box, box->tr->rbegin);
	ax_iter iter = box->tr->rbegin((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline ax_citer ax_box_crend(const ax_box* box)
{
	ax_require(box, box->tr->rend);
	ax_iter iter = box->tr->rend((ax_box *)box);
	return *ax_iter_c(&iter);
}

static inline void ax_box_clear(ax_box* box)
{
	ax_require(box, box->tr->clear);
	box->tr->clear(box);
}

static inline bool ax_box_iter_ended(ax_box* box, ax_iter *it)
{
	if (it->tr->norm)
		ax_require(box, box->tr->end);
	else
		ax_require(box, box->tr->rend);

	return ax_iter_equal(it, it->tr->norm
			? ax_ptrof(ax_iter, ax_box_end(box))
			: ax_ptrof(ax_iter, ax_box_rend(box)));
}

#endif
