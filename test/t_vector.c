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
#include "ax/vector.h"
#include "ax/algo.h"
#include "ut/suite.h"
#include "ut/runner.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(ut_runner *r)
{
	ax_vector_r vec1 = ax_new(ax_vector, ax_t(int));
	ut_assert(r, vec1.ax_any != NULL);
	ut_assert(r, ax_box_size(vec1.ax_box) == 0);

	ax_vector_r vec2 = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec2.ax_seq, ax_arraya(int, 1, 2, 3));
	int i = 1;
	ax_box_cforeach(vec2.ax_box, const int*, v) {
		ut_assert(r, *v == i++);
	}
	ax_one_free(vec1.ax_one);
	ax_one_free(vec2.ax_one);
}

static void push(ut_runner *r)
{

	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	for (int i = 0; i < 20; i++) {
		ax_seq_push(vec.ax_seq, &i);
	}

	for (int i = 0; i < 20; i++) {
		ax_iter it = ax_seq_at(vec.ax_seq, i);
		ut_assert(r, *(int*)ax_iter_get(&it) == i);
	}

	for (int i = 0; i < 20; i++) {
		ax_seq_pop(vec.ax_seq);
	}
	ut_assert(r, ax_box_size(vec.ax_box) == 0);

	ax_one_free(vec.ax_one);
}

static void iter(ut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_vector_r vec = ax_new(ax_vector, ax_t(int));

	for (i = 0; i < 20; i++) {
		ax_seq_push(vec.ax_seq, &i);
	}

	i = 0;
	ax_box_cforeach(vec.ax_box, const int*, v) {
		ut_assert(r, *v == i++);
	}

	i = 0;
	cur = ax_box_begin(vec.ax_box);
	last = ax_box_end(vec.ax_box);
	while (!ax_iter_equal(&cur, &last)) {
		ut_assert(r, *(int *)ax_iter_get(&cur) == i++);
		ax_iter_next(&cur);
	}

	ax_one_free(vec.ax_one);
}

static void riter(ut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	for (i = 0; i < 20; i++) {
		ax_seq_push(vec.ax_seq, &i);
	}

	i = 20 - 1;
	cur = ax_box_rbegin(vec.ax_box);
	last = ax_box_rend(vec.ax_box);
	while (!ax_iter_equal(&cur, &last)) {
		ut_assert(r, *(int *)ax_iter_get(&cur) == i--);
		ax_iter_next(&cur);
	}

	i = 20 - 1;
	cur = ax_box_end(vec.ax_box);
	last = ax_box_begin(vec.ax_box);
	do {
		ax_iter_prev(&cur);
		ut_assert(r, *(int *)ax_iter_get(&cur) == i--);
	} while (!ax_iter_equal(&cur, &last));

	i = 0;
	cur = ax_box_rend(vec.ax_box);
	last = ax_box_rbegin(vec.ax_box);
	do {
		ax_iter_prev(&cur);
		ut_assert(r, *(int *)ax_iter_get(&cur) == i++);
	} while (!ax_iter_equal(&cur, &last));

	ax_one_free(vec.ax_one);
}


static void seq_insert(ut_runner *r)
{
	int ins;

	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int, 1, 2));

	ax_iter it = ax_box_begin(vec.ax_box);

	ins = 3;
	ax_seq_insert(vec.ax_seq, &it, &ins);
	int table1[] = {3, 1, 2};
	ut_assert(r, seq_equal_array(vec.ax_seq, table1, sizeof table1));

	ins = 4;
	ax_seq_insert(vec.ax_seq, &it, &ins);
	int table2[] = {3, 4, 1, 2};
	ut_assert(r, seq_equal_array(vec.ax_seq, table2, sizeof table2));

	it = ax_box_end(vec.ax_box);
	ins = 5;
	ax_seq_insert(vec.ax_seq, &it, &ins);
	int table3[] = {3, 4, 1, 2, 5};
	ut_assert(r, seq_equal_array(vec.ax_seq, table3, sizeof table3));

	ax_one_free(vec.ax_one);
}

static void seq_insert_for_riter(ut_runner *r)
{
	int ins;

	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int, 2, 1));

	ax_iter it = ax_box_rbegin(vec.ax_box);

	ins = 3;
	ax_seq_insert(vec.ax_seq, &it, &ins);
	int table1[] = {2, 1, 3};
	ut_assert(r, seq_equal_array(vec.ax_seq, table1, sizeof table1));

	ins = 4;
	ax_seq_insert(vec.ax_seq, &it, &ins);
	int table2[] = {2, 1, 4, 3};
	ut_assert(r, seq_equal_array(vec.ax_seq, table2, sizeof table2));

	it = ax_box_rend(vec.ax_box);
	ins = 5;
	ax_seq_insert(vec.ax_seq, &it, &ins);
	int table3[] = {5, 2, 1, 4, 3};
	ut_assert(r, seq_equal_array(vec.ax_seq, table3, sizeof table3));

	ax_one_free(vec.ax_one);
}

static void any_copy(ut_runner *r)
{

	ax_vector_r vec1 = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec1.ax_seq, ax_arraya(int, 1, 2, 3, 4));
	ax_vector_r vec2 = AX_R_INIT(ax_any, ax_any_copy(vec1.ax_any));

	int table1[] = {1, 2, 3, 4};
	ut_assert(r, seq_equal_array(vec1.ax_seq, table1, sizeof table1));
	ut_assert(r, seq_equal_array(vec2.ax_seq, table1, sizeof table1));

	ax_box_clear(vec1.ax_box);

	ax_vector_r vec3 = AX_R_INIT(ax_any, ax_any_copy(vec1.ax_any));
	ut_assert(r, ax_box_size(vec3.ax_box) == 0);

	ax_one_free(vec1.ax_one);
	ax_one_free(vec2.ax_one);
	ax_one_free(vec3.ax_one);
}

static void seq_trunc(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int, 1, 2, 3));

	int table1[] = {1, 2, 3, 0, 0};
	ax_seq_trunc(vec.ax_seq, 5);
	ut_assert(r, seq_equal_array(vec.ax_seq, table1, sizeof table1));

	ax_seq_trunc(vec.ax_seq, 0);
	ut_assert(r, seq_equal_array(vec.ax_seq, NULL, 0));

	int table3[] = {0, 0, 0, 0, 0};
	ax_seq_trunc(vec.ax_seq, 5);
	ut_assert(r, seq_equal_array(vec.ax_seq, table3, sizeof table3));

	ax_one_free(vec.ax_one);
}

static void seq_invert(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int, 1, 2, 3, 4, 5));

	int table1[] = {5, 4, 3, 2, 1};
	ax_seq_invert(vec.ax_seq);
	ut_assert(r, seq_equal_array(vec.ax_seq, table1, sizeof table1));

	ax_box_clear(vec.ax_box);
	ut_assert(r, ax_box_size(vec.ax_box) == 0);

	ax_seq_invert(vec.ax_seq);
	ut_assert(r, ax_box_size(vec.ax_box) == 0);

	int table3[] = {1};
	ax_seq_push(vec.ax_seq, table3);
	ax_seq_invert(vec.ax_seq);
	ut_assert(r, seq_equal_array(vec.ax_seq, table3, sizeof table3));

	ax_one_free(vec.ax_one);
}

ut_suite *suite_for_vector()
{
	ut_suite* suite = ut_suite_create("vector");

	ut_suite_add(suite, create, 0);
	ut_suite_add(suite, push, 0);
	ut_suite_add(suite, any_copy, 0);
	ut_suite_add(suite, iter, 0);
	ut_suite_add(suite, riter, 0);
	ut_suite_add(suite, seq_insert, 0);
	ut_suite_add(suite, seq_insert_for_riter, 0);
	ut_suite_add(suite, seq_trunc, 0);
	ut_suite_add(suite, seq_invert, 0);

	return suite;
}
