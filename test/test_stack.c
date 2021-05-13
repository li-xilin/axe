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

#include "ax/stack.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_stack_r stack = ax_stack_create(ax_base_local(base), ax_stuff_traits(AX_ST_I));
	axut_assert_uint_equal(r, 0, ax_tube_size(stack.tube));
}

static void operate(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_stack_r stack = ax_stack_create(ax_base_local(base), ax_stuff_traits(AX_ST_I));
	int value;

	value = 1;
	ax_tube_push(stack.tube, &value);
	axut_assert_int_equal(r, 1, *(int *)ax_tube_prime(stack.tube));
	axut_assert_uint_equal(r, 1, ax_tube_size(stack.tube));

	value = 2;
	ax_tube_push(stack.tube, &value);
	axut_assert_int_equal(r, 2, *(int *)ax_tube_prime(stack.tube));
	axut_assert_uint_equal(r, 2, ax_tube_size(stack.tube));

	ax_tube_pop(stack.tube);
	axut_assert_int_equal(r, 1, *(int *)ax_tube_prime(stack.tube));
	axut_assert_uint_equal(r, 1, ax_tube_size(stack.tube));

	ax_tube_pop(stack.tube);
	axut_assert_uint_equal(r, 0, ax_tube_size(stack.tube));
}

static void clean(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_base_destroy(base);
}

axut_suite *suite_for_stack(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "stack");

	ax_base *base1 = ax_base_create();
	axut_suite_set_arg(suite, base1);

	axut_suite_add(suite, create, 0);
	axut_suite_add(suite, operate, 1);

	axut_suite_add(suite, clean, 0xFF);
	return suite;
}
