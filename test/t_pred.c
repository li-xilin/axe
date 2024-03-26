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

#include "ax/pred.h"
#include "ax/oper.h"
#include "ut/suite.h"
#include "ut/runner.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>

void binary(void *ret, const void *v1, const void *v2, void *ctx)
{
	*(int *)ret = (v1 ? *(int *)v1 : -1) + (v2 ? *(int *)v2 : -2);
}

void unary(void *ret, const void *v, void *ctx)
{
	*(int *)ret = (v ? *(int *)v : -1);
}


static void pred_binary(ut_runner *r)
{
	int n, ret, expect;
	ax_pred0 *p0;
	ax_pred1 *p1;
	ax_pred2 *p2;

	int _2 = 2, _3 = 3, _4 = 4, _5 = 5, _6 = 6, _7 = 7;

	ax_pred2 pred2 = ax_pred2_make(binary, &n);

	p2 = &pred2;
	ax_pred2_do(p2, &ret, &_2, &_3);
	binary(&expect, &_2, &_3, NULL);
	ut_assert_int_equal(r, expect, ret);

	p1 = ax_pred2_bind1(p2, &_4);
	ax_pred1_do(p1, &ret, &_5);
	binary(&expect, &_4, &_5, NULL);
	ut_assert_int_equal(r, expect, ret);

	p1 = ax_pred2_bind2(p2, &_4);
	ax_pred1_do(p1, &ret, &_5);
	binary(&expect, &_5, &_4, NULL);
	ut_assert_int_equal(r, expect, ret);

	p0 = ax_pred1_bind(p1, &_6);
	ax_pred0_do(p0, &ret);
	binary(&expect, &_6, &_4, NULL);
	ut_assert_int_equal(r, expect, ret);

	p1 = ax_pred2_bind2(p2, &_6);
	ax_pred1_do(p1, &ret, &_7);
	binary(&expect, &_7, &_6, NULL);
	ut_assert_int_equal(r, expect, ret);

	ax_pred2_do(p2, &ret, &_3, &_2);
	binary(&expect, &_3, &_2, NULL);
	ut_assert_int_equal(r, expect, ret);

}

static void pred_unary(ut_runner *r)
{

	int n, ret, expect;
	int _2 = 2, _3 = 3, _4 = 4;
	ax_pred0 *p0;
	ax_pred1 *p1;

	ax_pred1 pred1 = ax_pred1_make(unary, &n);

	p1 = &pred1;
	ax_pred1_do(p1, &ret, &_2);
	unary(&expect, &_2, NULL);
	ut_assert_int_equal(r, expect, ret);

	p0 = ax_pred1_bind(p1, &_3);
	ax_pred0_do(p0, &ret);
	unary(&expect, &_3, NULL);
	ut_assert_int_equal(r, expect, ret);

	ax_pred1_do(p1, &ret, &_4);
	unary(&expect, &_4, NULL);
	ut_assert_int_equal(r, expect, ret);

}

ut_suite *suite_for_pred()
{
	ut_suite *suite = ut_suite_create("pred");

	ut_suite_add(suite, pred_unary, 0);
	ut_suite_add(suite, pred_binary, 0);

	return suite;
}
