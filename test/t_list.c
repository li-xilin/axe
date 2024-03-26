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

#include "ax/iter.h"
#include "ax/list.h"
#include "ax/algo.h"
#include "ax/arraya.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool seq_equal_array(ax_seq *seq, void *arr, size_t mem_size)
{
	ax_box *box = ax_r(ax_seq, seq).ax_box;
	ax_iter first = ax_box_begin(box);
	ax_iter last = ax_box_end(box);
	return ax_equal_to_array(&first, &last, arr, mem_size, NULL);
}

static void create(ut_runner *r)
{
	ax_list_r list1 = ax_new(ax_list, ax_t(int));
	ut_assert(r, list1.ax_any != NULL);
	ut_assert(r, ax_box_size(list1.ax_box) == 0);

	ax_list_r list2 = ax_new(ax_list, ax_t(int));
	ax_seq_push_arraya(list2.ax_seq, ax_arraya(int, 1, 2, 3));

	int i = 1;
	ax_box_cforeach(list2.ax_box, const int*, v) {
		ut_assert(r, *v == i++);
	}

	ax_one_free(list1.ax_one);
	ax_one_free(list2.ax_one);
}

static void push(ut_runner *r)
{

	ax_list_r list = ax_new(ax_list, ax_t(int));
	for (int i = 0; i < 20; i++) {
		ax_seq_push(list.ax_seq, &i);
	}

	for (int i = 0; i < 20; i++) {
		ax_iter it = ax_seq_at(list.ax_seq, i);
		ut_assert(r, *(int*)ax_iter_get(&it) == i);
	}

	for (int i = 0; i < 20; i++) {
		ax_seq_pop(list.ax_seq);
	}
	ut_assert(r, ax_box_size(list.ax_box) == 0);

	ax_one_free(list.ax_one);
}

static void iter(ut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_list_r list = ax_new(ax_list, ax_t(int));

	for (i = 0; i < 20; i++) {
		ax_seq_push(list.ax_seq, &i);
	}

	i = 0;
	ax_box_cforeach(list.ax_box, const int*, v) {
		ut_assert(r, *v == i++);
	}

	i = 0;
	cur = ax_box_begin(list.ax_box);
	last = ax_box_end(list.ax_box);
	while (!ax_iter_equal(&cur, &last)) {
		ut_assert(r, *(int *)ax_iter_get(&cur) == i++);
		ax_iter_next(&cur);
	}


	ax_one_free(list.ax_one);
}

static void riter(ut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_list_r list = ax_new(ax_list, ax_t(int));
	for (i = 0; i < 20; i++) {
		ax_seq_push(list.ax_seq, &i);
	}

	i = 20 - 1;
	cur = ax_box_rbegin(list.ax_box);
	last = ax_box_rend(list.ax_box);
	while (!ax_iter_equal(&cur, &last)) {
		ut_assert(r, *(int *)ax_iter_get(&cur) == i--);
		ax_iter_next(&cur);
	}

	i = 20 - 1;
	cur = ax_box_end(list.ax_box);
	last = ax_box_begin(list.ax_box);
	do {
		ax_iter_prev(&cur);
		ut_assert(r, *(int *)ax_iter_get(&cur) == i--);
	} while (!ax_iter_equal(&cur, &last));

	i = 0;
	cur = ax_box_rend(list.ax_box);
	last = ax_box_rbegin(list.ax_box);
	do {
		ax_iter_prev(&cur);
		ut_assert(r, *(int *)ax_iter_get(&cur) == i++);
	} while (!ax_iter_equal(&cur, &last));

	ax_one_free(list.ax_one);
}


static void seq_insert(ut_runner *r)
{
	int ins;

	ax_list_r list = ax_new(ax_list, ax_t(int));
	ax_seq_push_arraya(list.ax_seq, ax_arraya(int, 1, 2));

	ax_iter it = ax_box_begin(list.ax_box);

	ins = 3;
	ax_seq_insert(list.ax_seq, &it, &ins);
	int table1[] = {3, 1, 2};
	ut_assert(r, seq_equal_array(list.ax_seq, table1, sizeof table1));

	ins = 4;
	ax_seq_insert(list.ax_seq, &it, &ins);
	int table2[] = {3, 4, 1, 2};
	ut_assert(r, seq_equal_array(list.ax_seq, table2, sizeof table2));

	it = ax_box_end(list.ax_box);
	ins = 5;
	ax_seq_insert(list.ax_seq, &it, &ins);
	int table3[] = {3, 4, 1, 2, 5};
	ut_assert(r, seq_equal_array(list.ax_seq, table3, sizeof table3));

	ax_one_free(list.ax_one);
}


static void seq_insert_for_riter(ut_runner *r)
{
	int ins;
	ax_list_r list = ax_new(ax_list, ax_t(int));
	ax_seq_push_arraya(list.ax_seq, ax_arraya(int, 2, 1));

	ax_iter it = ax_box_rbegin(list.ax_box);

	ins = 3;
	ax_seq_insert(list.ax_seq, &it, &ins);
	int table1[] = {2, 1, 3};
	ut_assert(r, seq_equal_array(list.ax_seq, table1, sizeof table1));

	ins = 4;
	ax_seq_insert(list.ax_seq, &it, &ins);
	int table2[] = {2, 1, 4, 3};
	ut_assert(r, seq_equal_array(list.ax_seq, table2, sizeof table2));

	it = ax_box_rend(list.ax_box);
	ins = 5;
	ax_seq_insert(list.ax_seq, &it, &ins);
	int table3[] = {5, 2, 1, 4, 3};
	ut_assert(r, seq_equal_array(list.ax_seq, table3, sizeof table3));

	ax_one_free(list.ax_one);
}

static void any_copy(ut_runner *r)
{

	ax_list_r list1 = ax_new(ax_list, ax_t(int));
	ax_seq_push_arraya(list1.ax_seq, ax_arraya(int, 1, 2, 3, 4));
	ax_list_r list2 = AX_R_INIT(ax_any, ax_any_copy(list1.ax_any));

	int table1[] = {1, 2, 3, 4};
	ut_assert(r, seq_equal_array(list1.ax_seq, table1, sizeof table1));
	ut_assert(r, seq_equal_array(list2.ax_seq, table1, sizeof table1));

	ax_box_clear(list1.ax_box);

	ax_list_r list3 = AX_R_INIT(ax_any, ax_any_copy(list1.ax_any));
	ut_assert(r, ax_box_size(list3.ax_box) == 0);

	ax_one_free(list1.ax_one);
	ax_one_free(list2.ax_one);
	ax_one_free(list3.ax_one);
}

static void seq_trunc(ut_runner *r)
{
	ax_list_r list = ax_new(ax_list, ax_t(int));
	ax_seq_push_arraya(list.ax_seq, ax_arraya(int, 1, 2, 3));

	int table1[] = {1, 2, 3, 0, 0};
	ax_seq_trunc(list.ax_seq, 5);
	ut_assert(r, seq_equal_array(list.ax_seq, table1, sizeof table1));

	ax_seq_trunc(list.ax_seq, 0);
	ut_assert(r, seq_equal_array(list.ax_seq, NULL, 0));

	int table3[] = {0, 0, 0, 0, 0};
	ax_seq_trunc(list.ax_seq, 5);
	ut_assert(r, seq_equal_array(list.ax_seq, table3, sizeof table3));

	ax_one_free(list.ax_one);
}

static void iter_erase(ut_runner *r)
{

	ax_list_r list = ax_new(ax_list, ax_t(int));
	ax_seq_push_arraya(list.ax_seq, ax_arraya(int, 1, 2, 3));

	ax_iter cur = ax_box_begin(list.ax_box),
		end = ax_box_end(list.ax_box);
	while (!ax_iter_equal(&cur, &end)) {
		ax_iter_erase(&cur);
	}

	ut_assert_uint_equal(r, 0, ax_box_size(list.ax_box));

	ax_one_free(list.ax_one);
}

static void seq_invert(ut_runner *r)
{
	ax_list_r list = ax_new(ax_list, ax_t(int));
	ax_seq_push_arraya(list.ax_seq, ax_arraya(int, 1, 2, 3, 4, 5));

	int table1[] = {5, 4, 3, 2, 1};
	ax_seq_invert(list.ax_seq);
	ut_assert(r, seq_equal_array(list.ax_seq, table1, sizeof table1));

	ax_box_clear(list.ax_box);
	ut_assert(r, ax_box_size(list.ax_box) == 0);

	ax_seq_invert(list.ax_seq);
	ut_assert(r, ax_box_size(list.ax_box) == 0);

	int table3[] = {1};
	ax_seq_push(list.ax_seq, table3);
	ax_seq_invert(list.ax_seq);
	ut_assert(r, seq_equal_array(list.ax_seq, table3, sizeof table3));

	ax_one_free(list.ax_one);
}

ut_suite* suite_for_list()
{
	ut_suite *suite = ut_suite_create("list");

	ut_suite_add(suite, create, 0);
	ut_suite_add(suite, push, 0);
	ut_suite_add(suite, iter, 0);
	ut_suite_add(suite, riter, 0);
	ut_suite_add(suite, seq_insert, 0);
	ut_suite_add(suite, seq_insert_for_riter, 0);
	ut_suite_add(suite, seq_trunc, 0);
	ut_suite_add(suite, seq_invert, 0);
	ut_suite_add(suite, any_copy, 0);
	ut_suite_add(suite, iter_erase, 0);

	return suite;
}
