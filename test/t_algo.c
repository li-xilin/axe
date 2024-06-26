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
#include "ax/algo.h"
#include "ax/iter.h"
#include "ax/vector.h"
#include "ax/list.h"
#include "ax/pred.h"
#include "ax/oper.h"
#include "ax/arraya.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void all_any_none_of(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, &ax_t_i32);

	for (int i = 0; i < 18; i++) { ax_seq_push(vec.ax_seq, &i); }
	ax_iter first = ax_box_begin(vec.ax_box), last = ax_box_end(vec.ax_box);
	int one = 1;
	ax_pred2 pred = ax_pred2_make(ax_op(int32_t).o_add, NULL);
	ax_transform(ax_iter_c(&first), ax_iter_c(&last), &first, ax_pred2_bind2(&pred, &one));

	int i = 0;
	ax_box_cforeach(vec.ax_box, const int *, p) {
		ut_assert(r, *p == i + 1);
		i++;
	}

	int twenty = 20;
	bool ret;
	pred = ax_pred2_make(ax_op(int32_t).o_lt, NULL);

	ret = ax_all_of(ax_iter_c(&first), ax_iter_c(&last), ax_pred2_bind2(&pred, &twenty));
	ut_assert(r, ret == true);

	ret = ax_any_of(ax_iter_c(&first), ax_iter_c(&last), ax_pred2_bind2(&pred, &twenty));
	ut_assert(r, ret == true);

	ret = ax_none_of(ax_iter_c(&first), ax_iter_c(&last), ax_pred2_bind2(&pred, &twenty));
	ut_assert(r, ret == false);

	ax_one_free(vec.ax_one);
}

static void partation(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int, 1, 8, 2, 4, 9, 5, 3, 6, 7));

	ax_iter first = ax_box_begin(vec.ax_box);
	ax_iter last = ax_box_end(vec.ax_box);
	int pivot = 5;
	ax_pred2 pred = ax_pred2_make(ax_op(int32_t).o_lt, NULL);
	
	ax_iter middle = first;
	ax_partition(&middle, &last, ax_pred2_bind2(&pred, &pivot));

	while(!ax_iter_equal(&first, &middle)) {
		int *val = ax_iter_get(&first);
		ut_assert(r, *val < pivot);
		ax_iter_next(&first);
	}

	while(!ax_iter_equal(&middle, &last)) {
		int *val = ax_iter_get(&middle);
		ut_assert(r, *val >= pivot);
		ax_iter_next(&middle);
	}

	ax_one_free(vec.ax_one);
}

static void quick_sort(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, &ax_t_i32);
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int32_t, 1, 8, 2, 4, 9, 5, 3, 6, 7, 0));

	ax_iter first = ax_box_begin(vec.ax_box);
	ax_iter last = ax_box_end(vec.ax_box);
	ax_pred2 pred = ax_pred2_make(ax_oper_uint32_t.o_le, NULL);
	ax_quick_sort(&first, &last, &pred);
	
	int i = 0;
	ax_box_cforeach(vec.ax_box, const int *, v) {
		ut_assert(r, *v == i);
		i++;
	}

	ax_one_free(vec.ax_one);
}

static void merge(ut_runner *r)
{
	ax_vector_r vec1 = ax_new(ax_vector, &ax_t_i32);
	ax_seq_push_arraya(vec1.ax_seq, ax_arraya(int32_t, 1, 3, 5));

	ax_vector_r vec2 = ax_new(ax_vector, &ax_t_i32);
	ax_seq_push_arraya(vec2.ax_seq, ax_arraya(int32_t, 2, 2, 6));

	ax_vector_r vec3 = ax_new(ax_vector, &ax_t_i32);
	ax_seq_trunc(vec3.ax_seq, 6);

	ax_iter first1 = ax_box_begin(vec1.ax_box);
	ax_iter last1 = ax_box_end(vec1.ax_box);
	ax_iter first2 = ax_box_begin(vec2.ax_box);
	ax_iter last2 = ax_box_end(vec2.ax_box);
	ax_iter dest = ax_box_begin(vec3.ax_box);

	ax_pred2 pred = ax_pred2_make(ax_oper_uint32_t.o_le, NULL);
	ax_merge(ax_iter_c(&first1), ax_iter_c(&last1), ax_iter_c(&first2), ax_iter_c(&last2), &dest, &pred);

	int32_t table1[] = {1, 2, 2, 3, 5, 6};
	ut_assert(r, seq_equal_array(vec3.ax_seq, table1, sizeof table1));

	ax_one_free(vec1.ax_one);
	ax_one_free(vec2.ax_one);
	ax_one_free(vec3.ax_one);
}

static void merge_sort(ut_runner *r)
{

	ax_vector_r vec = ax_new(ax_vector, &ax_t_i32);
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int32_t, 1, 8, 2, 4, 9, 5, 3, 6, 7, 0));

	ax_iter first = ax_box_begin(vec.ax_box);
	ax_iter last = ax_box_end(vec.ax_box);
	ax_pred2 pred = ax_pred2_make(ax_oper_uint32_t.o_le, NULL);
	ax_merge_sort(&first, &last, &pred);
	
	int32_t table1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	ut_assert(r, seq_equal_array(vec.ax_seq, table1, sizeof table1));

	ax_one_free(vec.ax_one);
}

static void binary_search(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, &ax_t_i32);
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int32_t, 1, 2, 3, 4));

	ax_iter first = ax_box_begin(vec.ax_box);
	ax_iter last = ax_box_end(vec.ax_box);

	int num;
	ax_iter res;
	for (num = 1; num <= 4; num++) {
		res = first;
		ax_binary_search(ax_iter_c(&res), ax_iter_c(&last), &num);
		ut_assert(r, !ax_iter_equal(&res, &last));
		ut_assert(r, *(int*)ax_iter_get(&res) == num);
	}

	num = 0;
	res = first;
	ax_binary_search(ax_iter_c(&res), ax_iter_c(&last), &num);
	ut_assert(r, ax_iter_equal(&res, &last));

	ax_one_free(vec.ax_one);
}

static void binary_search_if_not(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, ax_t(int));
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int, 1, 3, 5, 7));

	ax_citer first = ax_box_cbegin(vec.ax_box);
	ax_citer last = ax_box_cend(vec.ax_box);

	int num;
	ax_citer res;

	for (num = 0; num <= 6; num+=2) {
		ax_pred2 pred = ax_pred2_make(ax_oper_int32_t.o_le, NULL);
		res = first;
		ax_binary_search_if_not(&res, &last, ax_pred2_bind2(&pred, &num));
		ut_assert(r, !ax_citer_equal(&res, &last));
		ut_assert(r, *(int *)ax_citer_get(&res) == num+1);
	}

	num = 7;
	ax_pred2 pred = ax_pred2_make(ax_oper_int32_t.o_le, NULL);
	res = first;
	ax_binary_search_if_not(&res, &last, ax_pred2_bind2(&pred, &num));
	ut_assert(r, ax_citer_equal(&res, &last));

	num = 0;
	pred = ax_pred2_make(ax_oper_int32_t.o_le, NULL);
	res = first;
	ax_binary_search_if_not(&first, &last, ax_pred2_bind2(&pred, &num));
	ut_assert(r, !ax_citer_equal(&res, &last));
	ut_assert(r, *(int *)ax_citer_get(&res) == 1);

	ax_one_free(vec.ax_one);
}


static void insertion_sort(ut_runner *r)
{
	ax_vector_r vec = ax_new(ax_vector, &ax_t_i32);
	ax_seq_push_arraya(vec.ax_seq, ax_arraya(int32_t, 1, 8, 2, 4, 9, 5, 3, 6, 7, 0));

	ax_iter first = ax_box_begin(vec.ax_box);
	ax_iter last = ax_box_end(vec.ax_box);
	ax_pred2 pred = ax_pred2_make(ax_oper_int32_t.o_le, NULL);
	ax_insertion_sort(&first, &last, &pred);
	
	int32_t table1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	ut_assert(r, seq_equal_array(vec.ax_seq, table1, sizeof table1));

	ax_one_free(vec.ax_one);
}

static int qsort_compare_cb(const void* p1, const void* p2)
{
	const int *i1 = p1, *i2 = p2;
	return *i1 - *i2;

}

/**
 * history :
 * 0x3FFFFF 5.07, 6.88
 * 0x3FFFFF 4.24, 5.14
 * 0x3FFFFF 3.59, 4.01
 * 0x3FFFFF 3.64, 4.15
 * 0x3FFFFF 2.53, 3.01
 * 0x3FFFFF 1.96, 2.24
 * 0x3FFFFF 2.35, 2.27
 * 0x3FFFFF 2.42, 2.25
 * 0x3FFFFF 2.26, 2.27
 */

static void sort_time(ut_runner *r) {
#ifdef NDEBUG
	const size_t length = 0x3FFFFFF;
#else
	const size_t length = 0x3FFF;
#endif
	ax_iter first, last;
	clock_t time_before;
	srand(20);
	
	ax_vector_r vec1 = ax_new(ax_vector, &ax_t_i32),
		    vec2 = ax_new(ax_vector, &ax_t_i32),
		    vec3 = ax_new(ax_vector, &ax_t_i32),
		    vec4 = ax_new(ax_vector, &ax_t_i32);

	
	//printf("Sorting 0x%lX numbers\n", length);
	for (int i = 0; i < length; i++) {
		int n = rand() % 0xFFFFFF;
		ax_seq_push(vec1.ax_seq, &n);
		ax_seq_push(vec2.ax_seq, &n);
		ax_seq_push(vec3.ax_seq, &n);
		ax_seq_push(vec4.ax_seq, &n);
	}

	ax_pred2 pred = ax_pred2_make(ax_oper_int32_t.o_le, NULL);
	{
		first = ax_box_begin(vec1.ax_box);
		last = ax_box_end(vec1.ax_box);

		time_before = clock();
		ax_quick_sort(&first, &last, &pred);
		ut_printf(r, "ax_quick_sort() spent %lfs", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		ut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}

	{
		first = ax_box_begin(vec2.ax_box);
		last = ax_box_end(vec2.ax_box);

		time_before = clock();
		ax_merge_sort(&first, &last, &pred);
		ut_printf(r, "ax_merge_sort() spent %lfs", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		ut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}

#if 0 // Too slow
	{
		first = ax_box_begin(vec3.ax_box);
		last = ax_box_end(vec3.ax_box);

		time_before = clock();
		ax_insertion_sort(&first, &last, &pred);
		ut_printf(r, "ax_insertion_sort() spent %lfs", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		ut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}
#endif

	{
		time_before = clock();
		qsort(ax_vector_buffer(vec4.ax_vector), ax_box_size(vec4.ax_box), vec4.ax_box->env.elem_tr->t_size, qsort_compare_cb);
		printf("qsort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		ut_printf(r, "qsort() spent %lfs", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		ut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}
	
	ax_one_free(vec1.ax_one);
	ax_one_free(vec2.ax_one);
	ax_one_free(vec3.ax_one);
	ax_one_free(vec4.ax_one);
}

ut_suite *suite_for_algo()
{
	ut_suite* suite = ut_suite_create("algo");

	ut_suite_add(suite, all_any_none_of, 0);
	ut_suite_add(suite, partation, 0);
	ut_suite_add(suite, quick_sort, 0);
	ut_suite_add(suite, merge, 0);
	ut_suite_add(suite, merge_sort, 0);
	ut_suite_add(suite, sort_time, 0);
	ut_suite_add(suite, binary_search, 0);
	ut_suite_add(suite, binary_search_if_not, 0);
	ut_suite_add(suite, insertion_sort, 0);

	return suite;
}
