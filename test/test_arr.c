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
#include "ax/arr.h"
#include "ax/algo.h"
#include "ax/base.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(axut_runner *r)
{
	/*
	ax_base* base = ax_base_create();

	ax_base_destroy(base);
	return;
	ax_arr arr;
	int array[32];
	for (int i = 0; i < 32; i++) {
		array[i] = i;
	}
	ax_arr_r arr_r = ax_arr_make(&arr, base, ax_stuff_traits(AX_ST_I), array, 32);

	axut_assert(r, ax_box_size(arr_r.box) == 32);

	int j = 0;
	ax_box_cforeach(arr_r.box, const int*, v) {
		axut_assert(r, *v == j++);
	}

	ax_base_destroy(base);
	*/
}

#if 0

static void iter(axut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_base* base = ax_base_create();

	ax_vector_r vec_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	for (i = 0; i < 20; i++) {
		ax_seq_push(vec_r.seq, &i);
	}

	i = 0;
	ax_box_cforeach(vec_r.box, const int*, v) {
		axut_assert(r, *v == i++);
	}

	i = 0;
	cur = ax_box_begin(vec_r.box);
	last = ax_box_end(vec_r.box);
	while (!ax_iter_equal(&cur, &last)) {
		axut_assert(r, *(int32_t*)ax_iter_get(&cur) == i++);
		ax_iter_next(&cur);
	}

	ax_one_free(vec_r.one);
	ax_base_destroy(base);
}

static void riter(axut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_base* base = ax_base_create();
	ax_vector_r vec_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	for (i = 0; i < 20; i++) {
		ax_seq_push(vec_r.seq, &i);
	}

	i = 20 - 1;
	cur = ax_box_rbegin(vec_r.box);
	last = ax_box_rend(vec_r.box);
	while (!ax_iter_equal(&cur, &last)) {
		axut_assert(r, *(int32_t*)ax_iter_get(&cur) == i--);
		ax_iter_next(&cur);
	}

	i = 20 - 1;
	cur = ax_box_end(vec_r.box);
	last = ax_box_begin(vec_r.box);
	do {
		ax_iter_prev(&cur);
		axut_assert(r, *(int32_t*)ax_iter_get(&cur) == i--);
	} while (!ax_iter_equal(&cur, &last));

	i = 0;
	cur = ax_box_rend(vec_r.box);
	last = ax_box_rbegin(vec_r.box);
	do {
		ax_iter_prev(&cur);
		axut_assert(r, *(int32_t*)ax_iter_get(&cur) == i++);
	} while (!ax_iter_equal(&cur, &last));

	ax_one_free(vec_r.one);
	ax_base_destroy(base);
}

static void seq_trunc(axut_runner *r)
{
	ax_base* base = ax_base_create();

	ax_vector_r vec_r = ax_vector_init(ax_base_local(base), "i32x3", 1, 2, 3);

	int32_t table1[] = {1, 2, 3, 0, 0};
	ax_seq_trunc(vec_r.seq, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table1, sizeof table1));

	ax_seq_trunc(vec_r.seq, 0);
	axut_assert(r, seq_equal_array(vec_r.seq, NULL, 0));

	int32_t table3[] = {0, 0, 0, 0, 0};
	ax_seq_trunc(vec_r.seq, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table3, sizeof table3));

	ax_base_destroy(base);
}

static void seq_invert(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_vector_r vec_r = ax_vector_init(ax_base_local(base), "i32x5", 1, 2, 3, 4, 5);

	int32_t table1[] = {5, 4, 3, 2, 1};
	ax_seq_invert(vec_r.seq);
	axut_assert(r, seq_equal_array(vec_r.seq, table1, sizeof table1));

	ax_box_clear(vec_r.box);
	axut_assert(r, ax_box_size(vec_r.box) == 0);

	ax_seq_invert(vec_r.seq);
	axut_assert(r, ax_box_size(vec_r.box) == 0);

	int32_t table3[] = {1};
	ax_seq_push(vec_r.seq, table3);
	ax_seq_invert(vec_r.seq);
	axut_assert(r, seq_equal_array(vec_r.seq, table3, sizeof table3));

	ax_base_destroy(base);
}
#endif

axut_suite *suite_for_arr(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "arr");

	axut_suite_add(suite, create, 0);
	//axut_suite_add(suite, iter, 0);
	//axut_suite_add(suite, riter, 0);
	//axut_suite_add(suite, seq_trunc, 0);
	//axut_suite_add(suite, seq_invert, 0);

	return suite;
}
