/*
 * Copyright (c) 2022 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_PTRA_H
#define AX_PTRA_H

#include "type/one.h"
#include "trait.h"

#ifndef AX_PTRA_DEFINED
#define AX_PTRA_DEFINED
typedef struct ax_ptra_st ax_ptra;
#endif

#define ax_baseof_ax_ptra ax_one
ax_abstract_begin(ax_ptra)
	void *(*get)(ax_ptra* ptra);
	void *(*reset)(ax_ptra* ptra, void *ptr);
ax_end;

ax_concrete_begin(ax_ptra)
	const ax_trait *const tr;
	void *ptr;
	size_t nref;
ax_end;

ax_concrete(1, ax_ptra);

inline static void *ax_ptra_get(ax_ptra* ptra)
{
	return ptra->ptr;
}

inline static void ax_ptra_reset(ax_ptra* ptra, void *ptr)
{
	ptra->tr->free(ptra->ptr);
	ptra->ptr = ptr;
}

static void ax_ptra_free(ax_ptra *ptra)
{
	ptra->tr->free(ptra->ptr);
	ptra->ptr = NULL;
}

static const char *ax_ptra_name(const ax_ptra *ptra)
{
	return ax_class_name(1, ptra);
}

static const ax_one_trait ax_ptra_tr =
{
	.name = (ax_one_name_f)ax_ptra_name,
	.free = (ax_one_free_f)ax_ptra_free,
};

#define __ax_ptra_construct2(_tr, _ptr) \
	((struct ax_one_st *)(struct ax_ptra_st[1]) { \
			[0].one.tr = &ax_ptra_tr, \
		 	[0].tr = (_tr), \
			[0].nref = 1, \
			[0].ptr = _ptr, \
			})

#define ax_onelize_2(_p, _f) \
	ax_class_new_n(2, ptra, ax_p(const ax_trait, { .free = _f }), _p).one

#define ax_onelize_1(_p) \
	ax_onelize_2(_p, free)

#define ax_onelize(...) AX_OVERLOAD(ax_onelize_, __VA_ARGS__)

#endif
