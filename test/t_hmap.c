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

#include "ax/hmap.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 400

static void erase(ut_runner* r)
{
	ax_hmap_r hmap = ax_new(ax_hmap, ax_t(int), ax_t(int));

	for (int k = 0, v = 0; k < N; k++, v++) {
		int *ret = ax_map_put(hmap.ax_map, &k, &v);
		ut_assert(r, *ret == v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(hmap.ax_box), end = ax_box_end(hmap.ax_box);
	while (!ax_iter_equal(&it, &end)) {
		int *k = (int*)ax_map_iter_key(&it);
		int *v = (int*)ax_iter_get(&it);
		ut_assert(r, *k == *v);
		table[*k] = 1;
		ax_iter_next(&it);
	}

	for (int i = 0; i < N; i++) {
		ax_map_erase(hmap.ax_map, &i);
	}
	ut_assert_uint_equal(r, 0, ax_box_size(hmap.ax_box));

	for (int i = 0; i < N; i++) {
		ut_assert(r, table[i] == 1);
	}
	ax_one_free(hmap.ax_one);
}

static void iter_erase(ut_runner* r)
{

	ax_hmap_r hmap = ax_new(ax_hmap, ax_t(int), ax_t(int));
	for (int k = 0, v = 0; k < N; k++, v++) {
		int *ret = ax_map_put(hmap.ax_map, &k, &v);
		ut_assert(r, *ret == v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(hmap.ax_box),
		end = ax_box_end(hmap.ax_box);
	while(!ax_iter_equal(&it, &end)) {
		int *k = (int*)ax_map_iter_key(&it);
		int *v = (int*)ax_iter_get(&it);
		ut_assert_int_equal(r, *k, *v);
		table[*k] = 1;
		ax_iter_erase(&it);
		it = ax_box_begin(hmap.ax_box);
	}

	ut_assert_uint_equal(r, 0, ax_box_size(hmap.ax_box));

	for (int i = 0; i < N; i++) {
		ut_assert(r, table[i] == 1);
	}
	ax_one_free(hmap.ax_one);
}

static void map_chkey(ut_runner* r)
{

	ax_hmap_r hmap = ax_new(ax_hmap, ax_t(int), ax_t(int));
	int k, v;
	k = 1, v = 2;
	if (!ax_map_put(hmap.ax_map, &k, &v))
		ut_term(r, "ax_map_put");

	k = 3, v = 4;
	if (!ax_map_put(hmap.ax_map, &k, &v))
		ut_term(r, "ax_map_put");

	k = 1;
	int new = 5;
	int *kp = ax_map_chkey(hmap.ax_map, &k, &new);
	ut_assert_int_equal(r, 2, ax_box_size(hmap.ax_box));
	ut_assert(r, kp != NULL);
	ut_assert_int_equal(r, new, *kp);

	ut_assert_int_equal(r, 2, *(int *)ax_map_get(hmap.ax_map, &new));
	ax_one_free(hmap.ax_one);
}

static void duplicate(ut_runner *r)
{

	ax_hmap_r hmap = ax_new(ax_hmap, ax_t(int), ax_t(int));

	int key, val = 0;

	key = 1, val = 2;
	ax_map_put(hmap.ax_map, &key, &val);
	ut_assert_uint_equal(r, 1, ax_box_size(hmap.ax_box));
	
	key = 1, val = 3;
	ax_map_put(hmap.ax_map, &key, &val);
	ut_assert_uint_equal(r, 1, ax_box_size(hmap.ax_box));
	val = *(int *)ax_map_get(hmap.ax_map, &key);
	ut_assert_uint_equal(r, 3, val);

	key = 1, val = 4;
	ax_iter find = ax_map_at(hmap.ax_map, &key),
		end = ax_box_end(hmap.ax_box);
	ut_assert(r, !ax_iter_equal(&find, &end));
	ax_iter_set(&find, &val);
	ut_assert_uint_equal(r, 1, ax_box_size(hmap.ax_box));
	val = *(int *)ax_map_get(hmap.ax_map, &key);
	ut_assert_uint_equal(r, 4, val);


	key = 2, val = 5;
	ax_map_put(hmap.ax_map, &key, &val);
	ut_assert_uint_equal(r, 2, ax_box_size(hmap.ax_box));
	ax_map_erase(hmap.ax_map, &key);
	ut_assert_uint_equal(r, 1, ax_box_size(hmap.ax_box));
	ax_map_erase(hmap.ax_map, &key);
	ut_assert_uint_equal(r, 1, ax_box_size(hmap.ax_box));

	ax_one_free(hmap.ax_one);
}

static void iterate(ut_runner *r)
{

	ax_hmap_r hmap = ax_new(ax_hmap, ax_t(str), ax_t(int));
	const int count = 100;
	for (int i = 0; i < count; i++) {
		char key[4];
		sprintf(key, "%d", i);
		ax_map_put(hmap.ax_map, key, &i);
	}

	int check_table[count];
	for (int i = 0; i < count; i++) check_table[i] = -1;
	ax_map_cforeach(hmap.ax_map, const char *, key, const int *, val) {
		int k;
		sscanf(key, "%d", &k);
		ut_assert_int_equal(r, k, *val);
		check_table[k] = 0;
	}
	for (int i = 0; i < count; i++) 
		ut_assert(r, check_table[i] == 0);

	ax_one_free(hmap.ax_one);
}

static void rehash(ut_runner* r)
{
	ax_hmap_r hmap = ax_new(ax_hmap, ax_t(str), ax_t(int));
	const int count = 100;
	for (int i = 0; i < count; i++) {
		char key[4];
		sprintf(key, "%d", i);
		ax_map_put(hmap.ax_map, key, &i);
	}
	printf("threshold = %zu\n", ax_hmap_threshold(hmap.ax_hmap));
	if (ax_hmap_rehash(hmap.ax_hmap, 20))
		ut_term(r, "ax_hmap_rehash");

	int table[count];
	for (int i = 0; i < count; i++)
		table[i] = 0;


	ax_map_cforeach(hmap.ax_map, const char *, key, const int *, val) {
		ax_unused(key);
		table[*val] = 1;
	}
	for (int i = 0; i < count; i++)
		ut_assert(r, 1 == table[i]);

	ax_hmap_set_threshold(hmap.ax_hmap, 100);
	ut_assert_uint_equal(r, 100, ax_hmap_threshold(hmap.ax_hmap));

	ax_one_free(hmap.ax_one);
}


static void check_size(ut_runner* r)
{
	ax_hmap_r hmap = ax_new(ax_hmap, ax_t(int), ax_t(void));
	for (int i = 0; i < 10000; i++) {
		ax_map_put(hmap.ax_map, &i, NULL);
	}
	ut_assert_int_equal(r, 10000, ax_box_size(hmap.ax_box));
	ax_iter it = ax_box_begin(hmap.ax_box);
	size_t size = 0;
	ax_box_iterate(hmap.ax_box, it) {
		size += 1;
	}
	ut_assert_int_equal(r, 10000, size);

	for (int i = 0; i < 10000; i++) {
		ut_assert_int_equal(r, 10000 - i, ax_box_size(hmap.ax_box));
		ax_iter_erase(&it);
	}

	for (int i = 0; i < 10000; i++) {
		ax_map_put(hmap.ax_map, &i, NULL);
	}
	ut_assert_int_equal(r, 10000, ax_box_size(hmap.ax_box));

	for (int i = 0; i < 10000; i++) {
		ut_assert_int_equal(r, 10000 - i, ax_box_size(hmap.ax_box));
		it = ax_box_begin(hmap.ax_box);
		ax_map_erase(hmap.ax_map, ax_map_iter_key(&it));
	}
	ax_one_free(hmap.ax_one);
}

ut_suite *suite_for_hmap()
{
	ut_suite *suite = ut_suite_create("hmap");

	ut_suite_add(suite, iterate, 0);
	ut_suite_add(suite, erase, 1);
	ut_suite_add(suite, iter_erase, 1);
	ut_suite_add(suite, map_chkey, 1);
	ut_suite_add(suite, rehash, 1);
	ut_suite_add(suite, duplicate, 1);
	ut_suite_add(suite, check_size, 1);

	return suite;
}
