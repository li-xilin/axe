/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_PRED_H
#define AX_PRED_H
#include "debug.h"
#include "oper.h"
#include "def.h"

#define AX_BIND_1 1
#define AX_BIND_2 2
#define AX_BIND_U 4

#ifndef AX_PRED0_DEFINED
#define AX_PRED0_DEFINED
typedef struct ax_pred0_st ax_pred0;
#endif

#ifndef AX_PRED1_DEFINED
#define AX_PRED1_DEFINED
typedef struct ax_pred1_st ax_pred1;
#endif

#ifndef AX_PRED2_DEFINED
#define AX_PRED2_DEFINED
typedef struct ax_pred2_st ax_pred2;
#endif

#define __AX_PRED_FUNMASK (3 << 2)

#define __AX_PRED_BINDMASK (3)

#define __AX_PRED_BIND1 1
#define __AX_PRED_BIND2 2

#define __AX_PRED_FUN0 (0 << 2)
#define __AX_PRED_FUN1 (1 << 2)
#define __AX_PRED_FUN2 (2 << 2)

struct ax_pred0_st
{
	union {
		ax_unary_f unary;
		ax_binary_f binary;
	} p_fun;
	int p_bindf;
	void *p_ctx;
};

struct ax_pred1_st
{
	ax_pred0 p_p0;
	void *p_value;
};

struct ax_pred2_st
{
	ax_pred1 p_p1;
	void *p_value;
};


inline static ax_pred2 ax_pred2_make(ax_binary_f binary, void *ctx)
{
	return (ax_pred2) {
		.p_p1.p_p0.p_bindf = __AX_PRED_FUN2,
		.p_p1.p_p0.p_fun.binary = binary,
		.p_p1.p_p0.p_ctx = ctx,
	};
}


inline static ax_pred1 ax_pred1_make(ax_unary_f unary, void *ctx)
{
	return (ax_pred1) {
		.p_p0.p_bindf = __AX_PRED_FUN1,
		.p_p0.p_fun.unary = unary,
		.p_p0.p_ctx = ctx,
	};
}

inline static ax_pred0 ax_pred0_make(ax_unary_f unary, void *ctx)
{
	return (ax_pred0) {
		.p_bindf = __AX_PRED_FUN0,
		.p_fun.unary = unary,
		.p_ctx = ctx,
		
	};
}

inline static ax_pred1 *ax_pred2_bind1(ax_pred2 *p2, void *value)
{
	p2->p_p1.p_p0.p_bindf &= ~__AX_PRED_BINDMASK;
	p2->p_p1.p_p0.p_bindf |= __AX_PRED_BIND1;
	p2->p_value = value;
	return &p2->p_p1;
}

inline static ax_pred1 *ax_pred2_bind2(ax_pred2 *p2, void *value)
{
	p2->p_p1.p_p0.p_bindf &= ~__AX_PRED_BINDMASK;
	p2->p_p1.p_p0.p_bindf |= __AX_PRED_BIND2;
	p2->p_value = value;
	return &p2->p_p1;
}

inline static ax_pred0 *ax_pred1_bind(ax_pred1 *p1, void *value)
{
	p1->p_value = value;
	return &p1->p_p0;
}

inline static void *ax_pred2_do(ax_pred2 *p2, void *ret, const void *val1, const void *val2)
{
	ax_assert((p2->p_p1.p_p0.p_bindf & __AX_PRED_FUN2) == __AX_PRED_FUN2, "invalid function type of pred");
	p2->p_p1.p_p0.p_fun.binary(ret, val1, val2, p2->p_p1.p_p0.p_ctx);
	return ret;
}

inline static void *ax_pred1_do(ax_pred1 *p1, void *ret, const void *val)
{
	ax_pred2 *p2 = (ax_pred2 *)p1;
	switch (p1->p_p0.p_bindf) {
		case __AX_PRED_FUN1:
			p1->p_p0.p_fun.unary(ret, val, p1->p_p0.p_ctx);
			break;
		case __AX_PRED_FUN2 | __AX_PRED_BIND1:
			p1->p_p0.p_fun.binary(ret, p2->p_value, val, p1->p_p0.p_ctx);
			break;
		case __AX_PRED_FUN2 | __AX_PRED_BIND2:
			p1->p_p0.p_fun.binary(ret, val, p2->p_value, p1->p_p0.p_ctx);
			break;
		default:
			ax_assert(false, "invalid pred");
			break;
	}
	return ret;
}

inline static void* ax_pred0_do(ax_pred0 *p0, void *ret)
{
	ax_pred2 *p2 = (ax_pred2 *)p0;
	switch (p0->p_bindf) {
		case __AX_PRED_FUN0:
			p0->p_fun.unary(ret, NULL, p0->p_ctx);
		case __AX_PRED_FUN1:
			p2->p_p1.p_p0.p_fun.unary(ret, p2->p_p1.p_value, p0->p_ctx);
			break;
		case __AX_PRED_FUN2 | __AX_PRED_BIND1:
			p0->p_fun.binary(ret, p2->p_value, p2->p_p1.p_value, p2->p_p1.p_p0.p_ctx);
			break;
		case __AX_PRED_FUN2 | __AX_PRED_BIND2:
			p0->p_fun.binary(ret, p2->p_p1.p_value, p2->p_value, p2->p_p1.p_p0.p_ctx);
			break;
		default:
			ax_assert(false, "invalid pred");
	}
	return ret;
}

#endif
