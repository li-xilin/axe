#include "assist.h"

#include "axe/algo.h"
#include "axe/iter.h"
#include "axe/vector.h"
#include "axe/list.h"
#include "axe/pred.h"
#include "axe/oper.h"
#include "axe/base.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

static void all_any_none_of(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_vector_role role = ax_vector_create(
		ax_base_local(base),
		ax_stuff_traits(AX_ST_I32));
	for (int i = 0; i < 18; i++) { ax_seq_push(role.seq, &i); }
	ax_iter first = ax_box_begin(role.box), last = ax_box_end(role.box);
	int32_t one = 1;
	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I32)->add, NULL, &one, NULL);
	ax_transform(first, last, first, &pred);

	int i = 0;
	ax_foreach(int32_t *, p, role.box) {
		axut_assert(r, *p == i + 1);
		i++;
	}

	int32_t twenty = 20;
	ax_bool ret;
	pred = ax_pred_binary_make(ax_oper_for(AX_ST_I32)->lt, NULL, &twenty, NULL);

	ret = ax_all_of(first, last, &pred);
	axut_assert(r, ret == ax_true);

	ret = ax_any_of(first, last, &pred);
	axut_assert(r, ret == ax_true);

	ret = ax_none_of(first, last, &pred);
	axut_assert(r, ret == ax_false);

	ax_base_destroy(base);
}

static void partation(axut_runner *r)
{
	ax_base *base = ax_base_create();

	ax_vector_role role = ax_vector_init(ax_base_local(base),
			"i32x9", 1, 8, 2, 4, 9, 5, 3, 6, 7);
	ax_iter first = ax_box_begin(role.box);
	ax_iter last = ax_box_end(role.box);
	int32_t pivot = 5;
	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I32)->lt, NULL, &pivot, NULL);
	ax_iter middle = ax_partition(first, last, pred);

	while(!ax_iter_equal(first, middle)) {
		int32_t *val = ax_iter_get(first);
		axut_assert(r, *val < pivot);
		first = ax_iter_next(first);
	}

	while(!ax_iter_equal(middle, last)) {
		int32_t *val = ax_iter_get(middle);
		axut_assert(r, *val >= pivot);
		middle = ax_iter_next(middle);
	}

	ax_base_destroy(base);

}

static void quick_sort(axut_runner *r)
{
	ax_vector_role role;
	ax_base *base = ax_base_create();

	 role = ax_vector_init(ax_base_local(base),
			"i32x10", 1, 8, 2, 4, 9, 5, 3, 6, 7, 0);

	ax_iter first = ax_box_begin(role.box);
	ax_iter last = ax_box_end(role.box);
	ax_quick_sort(first, last);
	
	int32_t i = 0;
	ax_foreach(int32_t*, v, role.box) {
		axut_assert(r, *v == i);
		i++;
	}

	ax_base_destroy(base);
}

static void merge(axut_runner *r)
{
	ax_base *base = ax_base_create();

	ax_vector_role role1 = ax_vector_init(ax_base_local(base), "i32x3", 1, 3, 5);
	ax_vector_role role2 = ax_vector_init(ax_base_local(base), "i32x3", 2, 2, 6);
	ax_vector_role role3 = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_seq_trunc(role3.seq, 6);

	ax_iter first1 = ax_box_begin(role1.box);
	ax_iter last1 = ax_box_end(role1.box);
	ax_iter first2 = ax_box_begin(role2.box);
	ax_iter last2 = ax_box_end(role2.box);
	ax_iter dest = ax_box_begin(role3.box);
	ax_merge(first1, last1, first2, last2, dest);

	uint32_t table1[] = {1, 2, 2, 3, 5, 6};
	axut_assert(r, seq_equal_array(role3.seq, table1, sizeof table1));

	ax_base_destroy(base);

}

static void merge_sort(axut_runner *r)
{
	ax_vector_role role;
	ax_base *base = ax_base_create();

	 role = ax_vector_init(ax_base_local(base),
			"i32x10", 1, 8, 2, 4, 9, 5, 3, 6, 7, 0);

	ax_iter first = ax_box_begin(role.box);
	ax_iter last = ax_box_end(role.box);
	ax_merge_sort(first, last);
	
	uint32_t table1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	axut_assert(r, seq_equal_array(role.seq, table1, sizeof table1));

	ax_base_destroy(base);
}

static void binary_search(axut_runner *r)
{
	ax_vector_role role;
	ax_base *base = ax_base_create();

	role = ax_vector_init(ax_base_local(base),
			"i32x4", 1, 2, 3, 4);

	ax_iter first = ax_box_begin(role.box);
	ax_iter last = ax_box_end(role.box);

	uint32_t num;
	ax_iter res;

	for (num = 1; num <= 4; num++) {
		res = ax_binary_search(first, last, &num);
		axut_assert(r, !ax_iter_equal(res, last));
		axut_assert(r, *(uint32_t*)ax_iter_get(res) == num);
	}

	num = 0;
	res = ax_binary_search(first, last, &num);
	axut_assert(r, ax_iter_equal(res, last));

	ax_base_destroy(base);
}

static void binary_search_if_not(axut_runner *r)
{
	ax_vector_role role;
	ax_base *base = ax_base_create();

	role = ax_vector_init(ax_base_local(base),
			"i32x4", 1, 3, 5, 7);

	ax_iter first = ax_box_begin(role.box);
	ax_iter last = ax_box_end(role.box);

	uint32_t num;
	ax_iter res;

	for (num = 0; num <= 6; num+=2) {
		ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_U32)->le, NULL, &num, NULL);
		res = ax_binary_search_if_not(first, last, &pred);
		axut_assert(r, !ax_iter_equal(res, last));
		axut_assert(r, *(uint32_t*)ax_iter_get(res) == num+1);
	}

	num = 7;
	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_U32)->le, NULL, &num, NULL);
	res = ax_binary_search_if_not(first, last, &pred);
	axut_assert(r, ax_iter_equal(res, last));

	num = 0;
	pred = ax_pred_binary_make(ax_oper_for(AX_ST_U32)->le, NULL, &num, NULL);
	res = ax_binary_search_if_not(first, last, &pred);
	axut_assert(r, !ax_iter_equal(res, last));
	axut_assert(r, *(uint32_t*)ax_iter_get(res) == 1);

	ax_base_destroy(base);
}


static void insertion_sort(axut_runner *r)
{
	ax_vector_role role;
	ax_base *base = ax_base_create();

	role = ax_vector_init(ax_base_local(base),
			"i32x10", 1, 8, 2, 4, 9, 5, 3, 6, 7, 0);

	ax_iter first = ax_box_begin(role.box);
	ax_iter last = ax_box_end(role.box);
	ax_insertion_sort(first, last);
	
	uint32_t table1[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
	axut_assert(r, seq_equal_array(role.seq, table1, sizeof table1));

	ax_base_destroy(base);
}

static int qsort_compare_cb(const void* p1, const void* p2)
{
	const int32_t *i1 = p1, *i2 = p2;
	return *i1 - *i2;

}

static void sort_time(axut_runner *r) {
	const size_t length = 0xFFF;
	ax_iter first, last;
	clock_t time_before;
	srand(20);
	puts("history :");
	printf("\t0x3FFFFF 5.07, 6.88\n");
	printf("\t0x3FFFFF 4.24, 5.14\n");
	printf("\t0x3FFFFF 3.59, 4.01\n");
	printf("\t0x3FFFFF 3.64, 4.15\n");
	printf("\t0x3FFFFF 2.53, 3.01\n");

	ax_base *base = ax_base_create();
	ax_vector_role role1 = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_role role2 = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_role role3 = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_role role4 = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	
	printf("Sorting 0x%lX numbers\n", length);
	for (int i = 0; i < length; i++) {
		int32_t n = rand() % 0xFFFFFF;
		ax_seq_push(role1.seq, &n);
		ax_seq_push(role2.seq, &n);
		ax_seq_push(role3.seq, &n);
		ax_seq_push(role4.seq, &n);
	}

	ax_pred pred = ax_pred_binary_make(ax_oper_for(AX_ST_I32)->le, NULL, NULL, NULL);
	{
		first = ax_box_begin(role1.box);
		last = ax_box_end(role1.box);

		time_before = clock();
		ax_quick_sort(first, last);
		printf("ax_quick_sort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(first, last, &pred));
	}

	{
		first = ax_box_begin(role2.box);
		last = ax_box_end(role2.box);

		time_before = clock();
		ax_merge_sort(first, last);
		printf("ax_merge_sort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(first, last, &pred));
	}

	{
		first = ax_box_begin(role3.box);
		last = ax_box_end(role3.box);

		time_before = clock();
		ax_insertion_sort(first, last);
		printf("ax_insertion_sort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(first, last, &pred));
	}

	{
		time_before = clock();
		qsort(ax_vector_buffer(role4.vector), ax_box_size(role4.box), ax_box_elem_tr(role4.box)->size, qsort_compare_cb);
		printf("qsort() spent %lfs\n", (double)(clock()-time_before) / CLOCKS_PER_SEC );
		axut_assert(r, ax_sorted(first, last, &pred));
	}
	
	ax_base_destroy(base);

}

axut_suite *suite_for_algo(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "algo");

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
