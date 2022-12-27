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

#include "ut/runner.h"
#include "ut/suite.h"
#include "ax/avl.h"
#include "ax/iter.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 50

static void insert(ut_runner *r)
{
	ax_avl_r avl = ax_new(ax_avl, ax_t(i32), ax_t(i32));

	for (int k = 0, v = 0; k < N; k++, v++) {
		int32_t *ret = ax_map_put(avl.ax_map, &k, &v);
		ut_assert(r, *ret == v);

	}

	for (int k = 0; k < N; k++) {
		int *p = ax_map_get(avl.ax_map, &k);
		ut_assert(r, *p == k);
	}

	int i = 0;
	ax_map_foreach(avl.ax_map, const int32_t *, key, int32_t *, val) {
        ax_unused(val);
		 ut_assert(r, *key == i++);
	}

	i = N-1;
	ax_iter it = ax_box_rbegin(avl.ax_box), end = ax_box_rend(avl.ax_box);
	while (!ax_iter_equal(&it, &end)) {
		uint32_t *k = (uint32_t*)ax_map_iter_key(&it);
		ut_assert(r, *k == i);
		ax_iter_next(&it);
		i--;
	}

	ax_iter it_begin = ax_box_begin(avl.ax_box);
	ax_iter it_end = ax_box_end(avl.ax_box);
	ax_iter_prev(&it_end);
	ut_assert(r, ax_iter_dist(&it_begin, &it_end) == N-1);

	ax_one_free(avl.ax_one);
}

static void complex(ut_runner *r)
{

	ax_avl_r avl = ax_new(ax_avl, ax_t(i32), ax_t(i32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		ax_map_put(avl.ax_map, &k, &v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(avl.ax_box), end = ax_box_end(avl.ax_box);
	while (!ax_iter_equal(&it, &end)) {
		uint32_t *k = (uint32_t*)ax_map_iter_key(&it);
		uint32_t *v = (uint32_t*)ax_iter_get(&it);
		ut_assert(r, *k == *v);
		table[*k] = 1;
		ax_iter_next(&it);
	}

	for (int i = 0; i < N; i++) {
		ax_map_erase(avl.ax_map, &i);
	}
	ut_assert_uint_equal(r, 0, ax_box_size(avl.ax_box));

	for (int i = 0; i < N; i++) {
		ut_assert(r, table[i] == 1);
	}

	ax_one_free(avl.ax_one);
}

static void foreach(ut_runner *r)
{

	ax_avl_r avl = ax_new(ax_avl, ax_t(i32), ax_t(i32));
	int max = 50;
	for (int32_t i = 0; i < max; i++) {
		ax_map_put(avl.ax_map, &i, &i);
	}

	int count = 0;
	ax_map_foreach(avl.ax_map, const int *, k, int *, v) {
		ut_assert_uint_equal(r, *k, *v);
		count++;
	}
	ut_assert_int_equal(r, max, count);

	count = 0;
	ax_map_cforeach(avl.ax_map, const int *, k, const int *, v) {
        ax_unused(v);
		if (*k == 10)
			break;
		count++;
	}
	ut_assert_int_equal(r, 10, count);

	ax_one_free(avl.ax_one);
}

static void erase(ut_runner *r)
{

	ax_avl_r avl = ax_new(ax_avl, ax_t(i32), ax_t(i32));
	int count = 50;
	for (int32_t i = 0; i < count; i++) {
		ax_map_put(avl.ax_map, &i, &i);
	}

	ax_iter it = ax_box_begin(avl.ax_box), end = ax_box_end(avl.ax_box);
	while (!ax_iter_equal(&it, &end)) {
		ax_iter_erase(&it);
	}
	ut_assert_uint_equal(r, 0, ax_box_size(avl.ax_box));

	for (int32_t i = 0; i < count; i++) {
		ax_map_put(avl.ax_map, &i, &i);
	}

	for (int32_t i = 0; i < count; i++) {
		ax_map_erase(avl.ax_map, &i);
	}
	ut_assert_uint_equal(r, 0, ax_box_size(avl.ax_box));

	ax_one_free(avl.ax_one);
}

static void duplicate(ut_runner *r)
{

	ax_avl_r avl = ax_new(ax_avl, ax_t(i32), ax_t(i32));
	uint32_t key, val = 0;

	key = 1, val = 2;
	ax_map_put(avl.ax_map, &key, &val);
	ut_assert_uint_equal(r, 1, ax_box_size(avl.ax_box));
	
	key = 1, val = 3;
	ax_map_put(avl.ax_map, &key, &val);
	ut_assert_uint_equal(r, 1, ax_box_size(avl.ax_box));
	val = *(uint32_t *)ax_map_get(avl.ax_map, &key);
	ut_assert_uint_equal(r, 3, val);

	key = 1, val = 4;
	ax_iter find = ax_map_at(avl.ax_map, &key),
		end = ax_box_end(avl.ax_box);
	ut_assert(r, !ax_iter_equal(&find, &end));
	ax_iter_set(&find, &val);
	ut_assert_uint_equal(r, 1, ax_box_size(avl.ax_box));
	val = *(uint32_t *)ax_map_get(avl.ax_map, &key);
	ut_assert_uint_equal(r, 4, val);


	key = 2, val = 5;
	ax_map_put(avl.ax_map, &key, &val);
	ut_assert_uint_equal(r, 2, ax_box_size(avl.ax_box));
	ax_map_erase(avl.ax_map, &key);
	ut_assert_uint_equal(r, 1, ax_box_size(avl.ax_box));
	ax_map_erase(avl.ax_map, &key);
	ut_assert_uint_equal(r, 1, ax_box_size(avl.ax_box));

	ax_one_free(avl.ax_one);
}

static void clear(ut_runner *r)
{

	ax_avl_r avl = ax_new(ax_avl, ax_t(i32), ax_t(i32));
	for (int32_t i = 0; i < 50; i++) {
		ax_map_put(avl.ax_map, &i, &i);
	}
	ax_box_clear(avl.ax_box);
	ut_assert_uint_equal(r, ax_box_size(avl.ax_box), 0);

	ax_one_free(avl.ax_one);
}

ut_suite* suite_for_avl()
{
	ut_suite *suite = ut_suite_create("avl");

	ut_suite_add(suite, complex, 0);
	ut_suite_add(suite, insert, 0);
	ut_suite_add(suite, foreach, 0);
	ut_suite_add(suite, clear, 0);
	ut_suite_add(suite, duplicate, 0);
	ut_suite_add(suite, erase, 0);

	return suite;
}
