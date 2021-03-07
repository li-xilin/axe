#include "axut.h"

#include "axe/avl.h"
#include "axe/iter.h"
#include "axe/pair.h"
#include "axe/base.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 5000

static void insert(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_avl_role avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		ax_map_put(avl_r.map, &k, &v);
	}

	for (int k = 0; k < N; k++) {
		int *p = ax_map_get(avl_r.map, &k);
		axut_assert(r, *p == k);
	}

	int i = 0;
	ax_foreach(ax_pair *, pair, avl_r.box) {
		 axut_assert(r, *(int32_t*)ax_pair_key(pair) == i++);
	}

	i = N-1;
	ax_iter it = ax_box_rbegin(avl_r.box), end = ax_box_rend(avl_r.box);
	while (!ax_iter_equal(it, end)) {
		ax_pair *pair = ax_iter_get(it);
		int *k = (int*)ax_pair_key(pair);
		axut_assert(r, *k == i);
		it = ax_iter_next(it);
		i--;
	}

	ax_iter it_begin = ax_box_begin(avl_r.box);
	ax_iter it_end = ax_box_end(avl_r.box);
	it_end = ax_iter_prev(it_end);
	axut_assert(r, ax_iter_dist(it_begin, it_end) == N-1);

	ax_base_destroy(base);
}

static void complex(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_avl_role avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		ax_map_put(avl_r.map, &k, &v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(avl_r.box), end = ax_box_end(avl_r.box);
	while (!ax_iter_equal(it, end)) {
		ax_pair *pair = ax_iter_get(it);
		int *k = (int*)ax_pair_key(pair);
		int *v = (int*)ax_pair_value(pair);
		axut_assert(r, *k == *v);
		table[*k] = 1;
		it = ax_iter_next(it);
	}

	for (int i = 0; i < N; i++) {
		ax_map_erase(avl_r.map, &i);
	}
	axut_assert_uint_equal(r, 0, ax_box_size(avl_r.box));

	for (int i = 0; i < N; i++) {
		axut_assert(r, table[i] == 1);
	}
	ax_base_destroy(base);
}

static void foreach(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_avl_role avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int32_t i = 0; i < 50; i++) {
		ax_map_put(avl_r.map, &i, &i);
	}

	ax_iter it = ax_box_begin(avl_r.box), end = ax_box_end(avl_r.box);
	while (!ax_iter_equal(it, end)) {
		it = ax_iter_erase(it);
	}
	axut_assert_uint_equal(r, ax_box_size(avl_r.box), 0);

	ax_base_destroy(base);
}

axut_suite* suite_for_avl(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "avl");

	axut_suite_add(suite, complex, 0);
	axut_suite_add(suite, insert, 0);
	axut_suite_add(suite, foreach, 0);

	return suite;
}
