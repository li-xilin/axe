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

#include "one.h"
#include "trait.h"

#define AX_PTRA_NAME AX_ONE_NAME ".ptra"

#ifndef AX_PTRA_DEFINED
#define AX_PTRA_DEFINED
typedef struct ax_ptra_st ax_ptra;
#endif

#define AX_CLASS_BASE_ptra one
#define AX_CLASS_ROLE_ptra(_l) _l AX_CLASS_PTR(ptra); AX_CLASS_ROLE_one(_l)

AX_BEGIN_TRAIT(ptra)
	void *(*get)(ax_ptra* ptra);
	void *(*reset)(ax_ptra* ptra, void *ptr);
AX_END;

AX_BEGIN_ENV(ptra)
	const ax_trait *const tr;
	void *ptr;
	size_t nref;
AX_END;

AX_BLESS(ptra);

inline static void *ax_ptra_get(ax_ptra* ptra)
{
	return ptra->env.ptr;
}

inline static void ax_ptra_reset(ax_ptra* ptra, void *ptr)
{
	ptra->env.tr->free(ptra->env.ptr);
	ptra->env.ptr = ptr;
}

static void ax_ptra_free(ax_ptra* ptra)
{
	ptra->env.tr->free(ptra->env.ptr);
	ptra->env.ptr = NULL;
}

static const ax_ptra_trait ax_ptra_tr =
{
	.one = {
		.name = AX_PTRA_NAME,
		.free = (ax_one_free_f)ax_ptra_free,
	},
	.get = &ax_ptra_get,
};

#define __ax_ptra_construct2(_tr, _ptr) \
	((struct ax_one_st *)(struct ax_ptra_st[1]) { \
		 	[0].env.tr = (_tr), \
			[0].tr = &ax_ptra_tr, \
			[0].env.nref = 1, \
			[0].env.ptr = _ptr, \
			})

#define ax_onelize_2(_p, _f) \
	ax_class_new_n(2, ptra, ax_p(const ax_trait, { .free = _f }), _p).one

#define ax_onelize_1(_p) \
	ax_onelize_2(_p, free)

#define ax_onelize(...) AX_OVERLOAD(ax_onelize_, __VA_ARGS__)

#endif
