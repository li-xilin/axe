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

#include "assist.h"
#include "ax/iter.h"
#include "ax/array.h"
#include "ax/algo.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(ut_runner *r)
{
	int array[32];
	for (int i = 0; i < 32; i++)
		array[i] = i;

	ax_array_r arr1 = ax_new(ax_array, ax_t(int), array, 32 * sizeof(int));
	ax_array_r arr2 = ax_new(ax_array, ax_t(int), 32 * sizeof(int));

	for (int i = 0; i < 32; i++)
		((int *)ax_array_ptr(arr2.ax_array))[i] = i;

	ut_assert(r, ax_box_size(arr1.ax_box) == 32);
	ut_assert(r, ax_box_size(arr2.ax_box) == 32);

	int j = 0;
	ax_box_cforeach(arr1.ax_box, const int*, v) 
		ut_assert(r, *v == j++);

	j = 0;
	ax_box_cforeach(arr2.ax_box, const int*, v) {
		ut_assert(r, *v == j++);
	}
}

ut_suite *suite_for_arr()
{
	ut_suite* suite = ut_suite_create("arr");
	ut_suite_add(suite, create, 0);

	return suite;
}
