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
#include "axut.h"
#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void all_any_none_of(axut_runner *r)
{
	ax_vector_r vec = ax_class_new(vector, ax_t(int));

	for (int i = 0; i < 18; i++) { ax_seq_push(vec.seq, &i); }
	ax_iter first = ax_box_begin(vec.box), last = ax_box_end(vec.box);
	int one = 1;
	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I)->add, NULL, &one, NULL);
	ax_transform(ax_iter_c(&first), ax_iter_c(&last), &first, &pred);

	int i = 0;
	ax_box_cforeach(vec.box, const int *, p) {
		axut_assert(r, *p == i + 1);
		i++;
	}

	int twenty = 20;
	bool ret;
	pred = ax_pred_binary_make(ax_oper_for(AX_ST_I)->lt, NULL, &twenty, NULL);

	ret = ax_all_of(ax_iter_c(&first), ax_iter_c(&last), &pred);
	axut_assert(r, ret == true);

	ret = ax_any_of(ax_iter_c(&first), ax_iter_c(&last), &pred);
	axut_assert(r, ret == true);

	ret = ax_none_of(ax_iter_c(&first), ax_iter_c(&last), &pred);
	axut_assert(r, ret == false);

	ax_one_free(vec.one);
}

static void partation(axut_runner *r)
{
	ax_vector_r vec = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec.seq, ax_arraya(int, 1, 8, 2, 4, 9, 5, 3, 6, 7));

	ax_iter first = ax_box_begin(vec.box);
	ax_iter last = ax_box_end(vec.box);
	int pivot = 5;
	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I)->lt, NULL, &pivot, NULL);
	
	ax_iter middle = first;
	ax_partition(&middle, &last, &pred);

	while(!ax_iter_equal(&first, &middle)) {
		int *val = ax_iter_get(&first);
		axut_assert(r, *val < pivot);
		ax_iter_next(&first);
	}

	while(!ax_iter_equal(&middle, &last)) {
		int *val = ax_iter_get(&middle);
		axut_assert(r, *val >= pivot);
		ax_iter_next(&middle);
	}

	ax_one_free(vec.one);
}

static void quick_sort(axut_runner *r)
{
	ax_vector_r vec = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec.seq, ax_arraya(int, 1, 8, 2, 4, 9, 5, 3, 6, 7, 0));

	ax_iter first = ax_box_begin(vec.box);
	ax_iter last = ax_box_end(vec.box);
	ax_quick_sort(&first, &last);
	
	int i = 0;
	ax_box_cforeach(vec.box, const int *, v) {
		axut_assert(r, *v == i);
		i++;
	}

	ax_one_free(vec.one);
}

static void merge(axut_runner *r)
{
	ax_vector_r vec1 = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec1.seq, ax_arraya(int, 1, 3, 5));

	ax_vector_r vec2 = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec2.seq, ax_arraya(int, 2, 2, 6));

	ax_vector_r vec3 = ax_class_new(vector, ax_t(int));
	ax_seq_trunc(vec3.seq, 6);

	ax_iter first1 = ax_box_begin(vec1.box);
	ax_iter last1 = ax_box_end(vec1.box);
	ax_iter first2 = ax_box_begin(vec2.box);
	ax_iter last2 = ax_box_end(vec2.box);
	ax_iter dest = ax_box_begin(vec3.box);
	ax_merge(ax_iter_c(&first1), ax_iter_c(&last1), ax_iter_c(&first2), ax_iter_c(&last2), &dest);

	int table1[] = {1, 2, 2, 3, 5, 6};
	axut_assert(r, seq_equal_array(vec3.seq, table1, sizeof table1));

	ax_one_free(vec1.one);
	ax_one_free(vec2.one);
	ax_one_free(vec3.one);
}

static void merge_sort(axut_runner *r)
{

	ax_vector_r vec = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec.seq, ax_arraya(int, 1, 8, 2, 4, 9, 5, 3, 6, 7, 0));

	ax_iter first = ax_box_begin(vec.box);
	ax_iter last = ax_box_end(vec.box);
	ax_merge_sort(&first, &last);
	
	int table1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	axut_assert(r, seq_equal_array(vec.seq, table1, sizeof table1));

	ax_one_free(vec.one);
}

static void binary_search(axut_runner *r)
{
	ax_vector_r vec = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec.seq, ax_arraya(int, 1, 2, 3, 4));

	ax_iter first = ax_box_begin(vec.box);
	ax_iter last = ax_box_end(vec.box);

	int num;
	ax_iter res;
	for (num = 1; num <= 4; num++) {
		res = first;
		ax_binary_search(ax_iter_c(&res), ax_iter_c(&last), &num);
		axut_assert(r, !ax_iter_equal(&res, &last));
		axut_assert(r, *(int*)ax_iter_get(&res) == num);
	}

	num = 0;
	res = first;
	ax_binary_search(ax_iter_c(&res), ax_iter_c(&last), &num);
	axut_assert(r, ax_iter_equal(&res, &last));

	ax_one_free(vec.one);
}

static void binary_search_if_not(axut_runner *r)
{
	ax_vector_r vec = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec.seq, ax_arraya(int, 1, 3, 5, 7));

	ax_citer first = ax_box_cbegin(vec.box);
	ax_citer last = ax_box_cend(vec.box);

	int num;
	ax_citer res;

	for (num = 0; num <= 6; num+=2) {
		ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I)->le, NULL, &num, NULL);
		res = first;
		ax_binary_search_if_not(&res, &last, &pred);
		axut_assert(r, !ax_citer_equal(&res, &last));
		axut_assert(r, *(int *)ax_citer_get(&res) == num+1);
	}

	num = 7;
	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I)->le, NULL, &num, NULL);
	res = first;
	ax_binary_search_if_not(&res, &last, &pred);
	axut_assert(r, ax_citer_equal(&res, &last));

	num = 0;
	pred = ax_pred_binary_make(ax_oper_for(AX_ST_I)->le, NULL, &num, NULL);
	res = first;
	ax_binary_search_if_not(&first, &last, &pred);
	axut_assert(r, !ax_citer_equal(&res, &last));
	axut_assert(r, *(int *)ax_citer_get(&res) == 1);

	ax_one_free(vec.one);
}


static void insertion_sort(axut_runner *r)
{
	ax_vector_r vec = ax_class_new(vector, ax_t(int));
	ax_seq_push_arraya(vec.seq, ax_arraya(int, 1, 8, 2, 4, 9, 5, 3, 6, 7, 0));

	ax_iter first = ax_box_begin(vec.box);
	ax_iter last = ax_box_end(vec.box);
	ax_insertion_sort(&first, &last);
	
	int table1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	axut_assert(r, seq_equal_array(vec.seq, table1, sizeof table1));

	ax_one_free(vec.one);
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
 */

static void sort_time(axut_runner *r) {
	const size_t length = 0xFFF;
	//const size_t length = 0x3FFFFF;
	ax_iter first, last;
	clock_t time_before;
	srand(20);
	
	ax_vector_r vec1 = ax_class_new(vector, ax_t(int)),
		    vec2 = ax_class_new(vector, ax_t(int)),
		    vec3 = ax_class_new(vector, ax_t(int)),
		    vec4 = ax_class_new(vector, ax_t(int));

	
	//printf("Sorting 0x%lX numbers\n", length);
	for (int i = 0; i < length; i++) {
		int n = rand() % 0xFFFFFF;
		ax_seq_push(vec1.seq, &n);
		ax_seq_push(vec2.seq, &n);
		ax_seq_push(vec3.seq, &n);
		ax_seq_push(vec4.seq, &n);
	}

	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I)->le, NULL, NULL, NULL);
	{
		first = ax_box_begin(vec1.box);
		last = ax_box_end(vec1.box);

		time_before = clock();
		ax_quick_sort(&first, &last);
		printf("ax_quick_sort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}

	{
		first = ax_box_begin(vec2.box);
		last = ax_box_end(vec2.box);

		time_before = clock();
		ax_merge_sort(&first, &last);
		printf("ax_merge_sort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}

	/*
	{
		first = ax_box_begin(vec3.box);
		last = ax_box_end(vec3.box);

		time_before = clock();
		ax_insertion_sort(&first, &last);
		printf("ax_insertion_sort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}
	*/

	{
		time_before = clock();
		qsort(ax_vector_buffer(vec4.vector), ax_box_size(vec4.box), vec4.box->env.elem_tr->size, qsort_compare_cb);
		printf("qsort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(ax_iter_c(&first), ax_iter_c(&last), &pred));
	}

	
	ax_one_free(vec1.one);
	ax_one_free(vec2.one);
	ax_one_free(vec3.one);
	ax_one_free(vec4.one);
}

axut_suite *suite_for_algo()
{
	axut_suite* suite = axut_suite_create("algo");

	axut_suite_add(suite, all_any_none_of, 0);
	axut_suite_add(suite, partation, 0);
	axut_suite_add(suite, quick_sort, 0);
	axut_suite_add(suite, merge, 0);
	axut_suite_add(suite, merge_sort, 0);
	axut_suite_add(suite, sort_time, 0);
	axut_suite_add(suite, binary_search, 0);
	axut_suite_add(suite, binary_search_if_not, 0);
	axut_suite_add(suite, insertion_sort, 0);

	return suite;
}
