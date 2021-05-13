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

#include "axut.h"

#include "ax/avl.h"
#include "ax/iter.h"
#include "ax/base.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 50

static void insert(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_avl_r avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		int32_t *ret = ax_map_put(avl_r.map, &k, &v);
		axut_assert(r, *ret == v);

	}

	for (int k = 0; k < N; k++) {
		int *p = ax_map_get(avl_r.map, &k);
		axut_assert(r, *p == k);
	}

	int i = 0;
	ax_map_foreach(avl_r.map, const int32_t *, key, int32_t *, val) {
		 axut_assert(r, *key == i++);
	}

	i = N-1;
	ax_iter it = ax_box_rbegin(avl_r.box), end = ax_box_rend(avl_r.box);
	while (!ax_iter_equal(&it, &end)) {
		uint32_t *k = (uint32_t*)ax_map_iter_key(&it);
		axut_assert(r, *k == i);
		ax_iter_next(&it);
		i--;
	}

	ax_iter it_begin = ax_box_begin(avl_r.box);
	ax_iter it_end = ax_box_end(avl_r.box);
	ax_iter_prev(&it_end);
	axut_assert(r, ax_iter_dist(&it_begin, &it_end) == N-1);

}

static void complex(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_avl_r avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		ax_map_put(avl_r.map, &k, &v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(avl_r.box), end = ax_box_end(avl_r.box);
	while (!ax_iter_equal(&it, &end)) {
		uint32_t *k = (uint32_t*)ax_map_iter_key(&it);
		uint32_t *v = (uint32_t*)ax_iter_get(&it);
		axut_assert(r, *k == *v);
		table[*k] = 1;
		ax_iter_next(&it);
	}

	for (int i = 0; i < N; i++) {
		ax_map_erase(avl_r.map, &i);
	}
	axut_assert_uint_equal(r, 0, ax_box_size(avl_r.box));

	for (int i = 0; i < N; i++) {
		axut_assert(r, table[i] == 1);
	}
}

static void foreach(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_avl_r avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int32_t i = 0; i < 50; i++) {
		ax_map_put(avl_r.map, &i, &i);
	}

	ax_iter it = ax_box_begin(avl_r.box), end = ax_box_end(avl_r.box);
	while (!ax_iter_equal(&it, &end)) {
		ax_iter_erase(&it);
	}
	axut_assert_uint_equal(r, ax_box_size(avl_r.box), 0);

}

static void clear(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_avl_r avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int32_t i = 0; i < 50; i++) {
		ax_map_put(avl_r.map, &i, &i);
	}
	ax_box_clear(avl_r.box);
	axut_assert_uint_equal(r, ax_box_size(avl_r.box), 0);
}


static void clean(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_base_destroy(base);
}


axut_suite* suite_for_avl(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "avl");

	axut_suite_set_arg(suite, ax_base_create());

	axut_suite_add(suite, complex, 0);
	axut_suite_add(suite, insert, 0);
	axut_suite_add(suite, foreach, 0);
	axut_suite_add(suite, clear, 0);
	axut_suite_add(suite, clean, 0xFF);

	return suite;
}
