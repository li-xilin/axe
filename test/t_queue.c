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

#include "ax/queue.h"
#include "axut.h"
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(ax_runner *r)
{
	ax_queue_r queue = ax_new(queue, ax_t(int));
	axut_assert_uint_equal(r, 0, ax_tube_size(queue.tube));
	ax_one_free(queue.one);
}

static void operate(ax_runner *r)
{

	ax_queue_r queue = ax_new(queue, ax_t(int));
	int value;

	value = 1;
	ax_tube_push(queue.tube, &value);
	axut_assert_int_equal(r, 1, *(int *)ax_tube_prime(queue.tube));
	axut_assert_uint_equal(r, 1, ax_tube_size(queue.tube));

	value = 2;
	ax_tube_push(queue.tube, &value);
	axut_assert_int_equal(r, 1, *(int *)ax_tube_prime(queue.tube));
	axut_assert_uint_equal(r, 2, ax_tube_size(queue.tube));

	ax_tube_pop(queue.tube);
	axut_assert_int_equal(r, 2, *(int *)ax_tube_prime(queue.tube));
	axut_assert_uint_equal(r, 1, ax_tube_size(queue.tube));

	ax_tube_pop(queue.tube);
	axut_assert_uint_equal(r, 0, ax_tube_size(queue.tube));
	ax_one_free(queue.one);
}

axut_suite *suite_for_queue()
{
	axut_suite *suite = axut_suite_create("queue");

	axut_suite_add(suite, create, 0);
	axut_suite_add(suite, operate, 1);
	return suite;
}
