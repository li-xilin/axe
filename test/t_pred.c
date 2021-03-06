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
#include "axut/suite.h"
#include "axut/runner.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>

static void pred_unary(ax_runner *r)
{
	uint32_t in = 1, out;
	ax_pred pred = ax_pred_unary_make(ax_op(uint32_t).bit_not, NULL, NULL);
	ax_pred_do(&pred, &out, &in, NULL);
	axut_assert(r, out == 0xFFFFFFFE);

	bool bool_out;
	pred = ax_pred_unary_make(ax_op(uint32_t).not, &in, NULL);
	ax_pred_do(&pred, &bool_out, NULL, NULL);
	axut_assert(r, bool_out == false);

}

static void pred_binary(ax_runner *r)
{
	ax_pred pred;
	uint32_t in1 = 3, in2 = 4, out;

	pred = ax_pred_binary_make(ax_op(uint32_t).add, NULL, NULL, NULL);
	ax_pred_do(&pred, &out, &in1, &in2);
	axut_assert(r, out == 7);

	pred = ax_pred_binary_make(ax_op(uint32_t).add, &in1, NULL, NULL);
	ax_pred_do(&pred, &out, &in2, NULL);
	axut_assert(r, out == 7);

	pred = ax_pred_binary_make(ax_op(uint32_t).add, NULL, &in2, NULL);
	ax_pred_do(&pred, &out, &in1, NULL);
	axut_assert(r, out == 7);

	pred = ax_pred_binary_make(ax_op(uint32_t).add, &in1, &in2, NULL);
	ax_pred_do(&pred, &out, NULL, NULL);
	axut_assert(r, out == 7);

}

axut_suite *suite_for_pred()
{
	axut_suite *suite = axut_suite_create("pred");

	axut_suite_add(suite, pred_unary, 0);
	axut_suite_add(suite, pred_binary, 0);

	return suite;
}
