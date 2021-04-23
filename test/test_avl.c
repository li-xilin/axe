#include "axut.h"

#include "axe/avl.h"
#include "axe/iter.h"
#include "axe/base.h"

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
