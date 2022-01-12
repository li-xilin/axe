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

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static bool seq_equal_array(ax_seq *seq, void *arr, size_t mem_size)
{
	ax_box *box = ax_r(seq, seq).box;
	ax_iter first = ax_box_begin(box);
	ax_iter last = ax_box_end(box);
	return ax_equal_to_arr(&first, &last, arr, mem_size);
}

static void create(axut_runner *r)
{
	ax_list_r list1 = ax_class_new(list, ax_t(int));
	axut_assert(r, list1.any != NULL);
	axut_assert(r, ax_box_size(list1.box) == 0);

	ax_list_r list2 = ax_class_new(list, ax_t(int));
	ax_seq_push_arraya(list2.seq, ax_arraya(int, 1, 2, 3));

	int i = 1;
	ax_box_cforeach(list2.box, const int*, v) {
		axut_assert(r, *v == i++);
	}

	ax_one_free(list1.one);
	ax_one_free(list2.one);
}

static void push(axut_runner *r)
{

	ax_list_r list = ax_class_new(list, ax_t(int));
	for (int i = 0; i < 20; i++) {
		ax_seq_push(list.seq, &i);
	}

	for (int i = 0; i < 20; i++) {
		ax_iter it = ax_seq_at(list.seq, i);
		axut_assert(r, *(int*)ax_iter_get(&it) == i);
	}

	for (int i = 0; i < 20; i++) {
		ax_seq_pop(list.seq);
	}
	axut_assert(r, ax_box_size(list.box) == 0);

	ax_one_free(list.one);
}

static void iter(axut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_list_r list = ax_class_new(list, ax_t(int));

	for (i = 0; i < 20; i++) {
		ax_seq_push(list.seq, &i);
	}

	i = 0;
	ax_box_cforeach(list.box, const int*, v) {
		axut_assert(r, *v == i++);
	}

	i = 0;
	cur = ax_box_begin(list.box);
	last = ax_box_end(list.box);
	while (!ax_iter_equal(&cur, &last)) {
		axut_assert(r, *(int *)ax_iter_get(&cur) == i++);
		ax_iter_next(&cur);
	}


	ax_one_free(list.one);
}

static void riter(axut_runner *r)
{
	ax_iter cur, last;
	int i;

	ax_list_r list = ax_class_new(list, ax_t(int));
	for (i = 0; i < 20; i++) {
		ax_seq_push(list.seq, &i);
	}

	i = 20 - 1;
	cur = ax_box_rbegin(list.box);
	last = ax_box_rend(list.box);
	while (!ax_iter_equal(&cur, &last)) {
		axut_assert(r, *(int *)ax_iter_get(&cur) == i--);
		ax_iter_next(&cur);
	}

	i = 20 - 1;
	cur = ax_box_end(list.box);
	last = ax_box_begin(list.box);
	do {
		ax_iter_prev(&cur);
		axut_assert(r, *(int *)ax_iter_get(&cur) == i--);
	} while (!ax_iter_equal(&cur, &last));

	i = 0;
	cur = ax_box_rend(list.box);
	last = ax_box_rbegin(list.box);
	do {
		ax_iter_prev(&cur);
		axut_assert(r, *(int *)ax_iter_get(&cur) == i++);
	} while (!ax_iter_equal(&cur, &last));

	ax_one_free(list.one);
}


static void seq_insert(axut_runner *r)
{
	int ins;

	ax_list_r list = ax_class_new(list, ax_t(int));
	ax_seq_push_arraya(list.seq, ax_arraya(int, 1, 2));

	ax_iter it = ax_box_begin(list.box);

	ins = 3;
	ax_seq_insert(list.seq, &it, &ins);
	int table1[] = {3, 1, 2};
	axut_assert(r, seq_equal_array(list.seq, table1, sizeof table1));

	ins = 4;
	ax_seq_insert(list.seq, &it, &ins);
	int table2[] = {3, 4, 1, 2};
	axut_assert(r, seq_equal_array(list.seq, table2, sizeof table2));

	it = ax_box_end(list.box);
	ins = 5;
	ax_seq_insert(list.seq, &it, &ins);
	int table3[] = {3, 4, 1, 2, 5};
	axut_assert(r, seq_equal_array(list.seq, table3, sizeof table3));

	ax_one_free(list.one);
}


static void seq_insert_for_riter(axut_runner *r)
{
	int ins;
	ax_list_r list = ax_class_new(list, ax_t(int));
	ax_seq_push_arraya(list.seq, ax_arraya(int, 2, 1));

	ax_iter it = ax_box_rbegin(list.box);

	ins = 3;
	ax_seq_insert(list.seq, &it, &ins);
	int table1[] = {2, 1, 3};
	axut_assert(r, seq_equal_array(list.seq, table1, sizeof table1));

	ins = 4;
	ax_seq_insert(list.seq, &it, &ins);
	int table2[] = {2, 1, 4, 3};
	axut_assert(r, seq_equal_array(list.seq, table2, sizeof table2));

	it = ax_box_rend(list.box);
	ins = 5;
	ax_seq_insert(list.seq, &it, &ins);
	int table3[] = {5, 2, 1, 4, 3};
	axut_assert(r, seq_equal_array(list.seq, table3, sizeof table3));

	ax_one_free(list.one);
}

static void any_copy(axut_runner *r)
{

	ax_list_r list1 = ax_class_new(list, ax_t(int));
	ax_seq_push_arraya(list1.seq, ax_arraya(int, 1, 2, 3, 4));
	ax_list_r list2 = { .any = ax_any_copy(list1.any) };

	int table1[] = {1, 2, 3, 4};
	axut_assert(r, seq_equal_array(list1.seq, table1, sizeof table1));
	axut_assert(r, seq_equal_array(list2.seq, table1, sizeof table1));

	ax_box_clear(list1.box);

	ax_list_r list3 = { .any = ax_any_copy(list1.any) };
	axut_assert(r, ax_box_size(list3.box) == 0);

	ax_one_free(list1.one);
	ax_one_free(list2.one);
	ax_one_free(list3.one);
}

static void seq_trunc(axut_runner *r)
{
	ax_list_r list = ax_class_new(list, ax_t(int));
	ax_seq_push_arraya(list.seq, ax_arraya(int, 1, 2, 3));

	int table1[] = {1, 2, 3, 0, 0};
	ax_seq_trunc(list.seq, 5);
	axut_assert(r, seq_equal_array(list.seq, table1, sizeof table1));

	ax_seq_trunc(list.seq, 0);
	axut_assert(r, seq_equal_array(list.seq, NULL, 0));

	int table3[] = {0, 0, 0, 0, 0};
	ax_seq_trunc(list.seq, 5);
	axut_assert(r, seq_equal_array(list.seq, table3, sizeof table3));

	ax_one_free(list.one);
}

static void iter_erase(axut_runner *r)
{

	ax_list_r list = ax_class_new(list, ax_t(int));
	ax_seq_push_arraya(list.seq, ax_arraya(int, 1, 2, 3));

	ax_iter cur = ax_box_begin(list.box),
		end = ax_box_end(list.box);
	while (!ax_iter_equal(&cur, &end)) {
		ax_iter_erase(&cur);
	}

	axut_assert_uint_equal(r, 0, ax_box_size(list.box));

	ax_one_free(list.one);
}

static void seq_invert(axut_runner *r)
{
	ax_list_r list = ax_class_new(list, ax_t(int));
	ax_seq_push_arraya(list.seq, ax_arraya(int, 1, 2, 3, 4, 5));

	int table1[] = {5, 4, 3, 2, 1};
	ax_seq_invert(list.seq);
	axut_assert(r, seq_equal_array(list.seq, table1, sizeof table1));

	ax_box_clear(list.box);
	axut_assert(r, ax_box_size(list.box) == 0);

	ax_seq_invert(list.seq);
	axut_assert(r, ax_box_size(list.box) == 0);

	int table3[] = {1};
	ax_seq_push(list.seq, table3);
	ax_seq_invert(list.seq);
	axut_assert(r, seq_equal_array(list.seq, table3, sizeof table3));

	ax_one_free(list.one);
}

axut_suite* suite_for_list()
{
	axut_suite *suite = axut_suite_create("list");

	axut_suite_add(suite, create, 0);
	axut_suite_add(suite, push, 0);
	axut_suite_add(suite, iter, 0);
	axut_suite_add(suite, riter, 0);
	axut_suite_add(suite, seq_insert, 0);
	axut_suite_add(suite, seq_insert_for_riter, 0);
	axut_suite_add(suite, seq_trunc, 0);
	axut_suite_add(suite, seq_invert, 0);
	axut_suite_add(suite, any_copy, 0);
	axut_suite_add(suite, iter_erase, 0);

	return suite;
}
