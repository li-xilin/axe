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

#include "axut/runner.h"
#include "axut/suite.h"
#include "ax/avl.h"
#include "ax/iter.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 50

static void insert(ax_runner *r)
{
	ax_avl_r avl = ax_new(avl, ax_t(i32), ax_t(i32));

	for (int k = 0, v = 0; k < N; k++, v++) {
		int32_t *ret = ax_map_put(avl.map, &k, &v);
		axut_assert(r, *ret == v);

	}

	for (int k = 0; k < N; k++) {
		int *p = ax_map_get(avl.map, &k);
		axut_assert(r, *p == k);
	}

	int i = 0;
	ax_map_foreach(avl.map, const int32_t *, key, int32_t *, val) {
		 axut_assert(r, *key == i++);
	}

	i = N-1;
	ax_iter it = ax_box_rbegin(avl.box), end = ax_box_rend(avl.box);
	while (!ax_iter_equal(&it, &end)) {
		uint32_t *k = (uint32_t*)ax_map_iter_key(&it);
		axut_assert(r, *k == i);
		ax_iter_next(&it);
		i--;
	}

	ax_iter it_begin = ax_box_begin(avl.box);
	ax_iter it_end = ax_box_end(avl.box);
	ax_iter_prev(&it_end);
	axut_assert(r, ax_iter_dist(&it_begin, &it_end) == N-1);

	ax_one_free(avl.one);
}

static void complex(ax_runner *r)
{

	ax_avl_r avl = ax_new(avl, ax_t(i32), ax_t(i32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		ax_map_put(avl.map, &k, &v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(avl.box), end = ax_box_end(avl.box);
	while (!ax_iter_equal(&it, &end)) {
		uint32_t *k = (uint32_t*)ax_map_iter_key(&it);
		uint32_t *v = (uint32_t*)ax_iter_get(&it);
		axut_assert(r, *k == *v);
		table[*k] = 1;
		ax_iter_next(&it);
	}

	for (int i = 0; i < N; i++) {
		ax_map_erase(avl.map, &i);
	}
	axut_assert_uint_equal(r, 0, ax_box_size(avl.box));

	for (int i = 0; i < N; i++) {
		axut_assert(r, table[i] == 1);
	}

	ax_one_free(avl.one);
}

static void foreach(ax_runner *r)
{

	ax_avl_r avl = ax_new(avl, ax_t(i32), ax_t(i32));
	int max = 50;
	for (int32_t i = 0; i < max; i++) {
		ax_map_put(avl.map, &i, &i);
	}

	int count = 0;
	ax_map_foreach(avl.map, const int *, k, int *, v) {
		axut_assert_uint_equal(r, *k, *v);
		count++;
	}
	axut_assert_int_equal(r, max, count);

	count = 0;
	ax_map_cforeach(avl.map, const int *, k, const int *, v) {
		if (*k == 10)
			break;
		count++;
	}
	axut_assert_int_equal(r, 10, count);

	ax_one_free(avl.one);
}

static void erase(ax_runner *r)
{

	ax_avl_r avl = ax_new(avl, ax_t(i32), ax_t(i32));
	int count = 50;
	for (int32_t i = 0; i < count; i++) {
		ax_map_put(avl.map, &i, &i);
	}

	ax_iter it = ax_box_begin(avl.box), end = ax_box_end(avl.box);
	while (!ax_iter_equal(&it, &end)) {
		ax_iter_erase(&it);
	}
	axut_assert_uint_equal(r, 0, ax_box_size(avl.box));

	for (int32_t i = 0; i < count; i++) {
		ax_map_put(avl.map, &i, &i);
	}

	for (int32_t i = 0; i < count; i++) {
		ax_map_erase(avl.map, &i);
	}
	axut_assert_uint_equal(r, 0, ax_box_size(avl.box));

	ax_one_free(avl.one);
}

static void duplicate(ax_runner *r)
{

	ax_avl_r avl = ax_new(avl, ax_t(i32), ax_t(i32));
	uint32_t key, val = 0;

	key = 1, val = 2;
	ax_map_put(avl.map, &key, &val);
	axut_assert_uint_equal(r, 1, ax_box_size(avl.box));
	
	key = 1, val = 3;
	ax_map_put(avl.map, &key, &val);
	axut_assert_uint_equal(r, 1, ax_box_size(avl.box));
	val = *(uint32_t *)ax_map_get(avl.map, &key);
	axut_assert_uint_equal(r, 3, val);

	key = 1, val = 4;
	ax_iter find = ax_map_at(avl.map, &key),
		end = ax_box_end(avl.box);
	axut_assert(r, !ax_iter_equal(&find, &end));
	ax_iter_set(&find, &val);
	axut_assert_uint_equal(r, 1, ax_box_size(avl.box));
	val = *(uint32_t *)ax_map_get(avl.map, &key);
	axut_assert_uint_equal(r, 4, val);


	key = 2, val = 5;
	ax_map_put(avl.map, &key, &val);
	axut_assert_uint_equal(r, 2, ax_box_size(avl.box));
	ax_map_erase(avl.map, &key);
	axut_assert_uint_equal(r, 1, ax_box_size(avl.box));
	ax_map_erase(avl.map, &key);
	axut_assert_uint_equal(r, 1, ax_box_size(avl.box));

	ax_one_free(avl.one);
}

static void clear(ax_runner *r)
{

	ax_avl_r avl = ax_new(avl, ax_t(i32), ax_t(i32));
	for (int32_t i = 0; i < 50; i++) {
		ax_map_put(avl.map, &i, &i);
	}
	ax_box_clear(avl.box);
	axut_assert_uint_equal(r, ax_box_size(avl.box), 0);

	ax_one_free(avl.one);
}

axut_suite* suite_for_avl()
{
	axut_suite *suite = axut_suite_create("avl");

	axut_suite_add(suite, complex, 0);
	axut_suite_add(suite, insert, 0);
	axut_suite_add(suite, foreach, 0);
	axut_suite_add(suite, clear, 0);
	axut_suite_add(suite, duplicate, 0);
	axut_suite_add(suite, erase, 0);

	return suite;
}
