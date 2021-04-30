#include <axut.h>

#include <axe/hmap.h>
#include <axe/avl.h>
#include <axe.h>

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define N 400

static void map_erase(axut_runner* r)
{
	ax_base* base = axut_runner_arg(r);
	ax_hmap_r hmap_r = ax_hmap_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		int32_t *ret = ax_map_put(hmap_r.map, &k, &v);
		axut_assert(r, *ret == v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(hmap_r.box), end = ax_box_end(hmap_r.box);
	while (!ax_iter_equal(&it, &end)) {
		int *k = (int*)ax_map_iter_key(&it);
		int *v = (int*)ax_iter_get(&it);
		axut_assert(r, *k == *v);
		table[*k] = 1;
		ax_iter_next(&it);
	}

	for (int i = 0; i < N; i++) {
		ax_map_erase(hmap_r.map, &i);
	}
	axut_assert(r, ax_box_size(hmap_r.box) == 0);

	for (int i = 0; i < N; i++) {
		axut_assert(r, table[i] == 1);
	}
}

static void iter_erase(axut_runner* r)
{
	ax_base* base = axut_runner_arg(r);
	ax_hmap_r hmap_r = ax_hmap_create(ax_base_local(base), ax_stuff_traits(AX_ST_I),
			ax_stuff_traits(AX_ST_I));
	for (int k = 0, v = 0; k < N; k++, v++) {
		int32_t *ret = ax_map_put(hmap_r.map, &k, &v);
		axut_assert(r, *ret == v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(hmap_r.box),
		end = ax_box_end(hmap_r.box);
	int i = 0;
	while(!ax_iter_equal(&it, &end)) {
		int *k = (int*)ax_map_iter_key(&it);
		int *v = (int*)ax_iter_get(&it);
		axut_assert_int_equal(r, *k, *v);
		table[*k] = 1;
		ax_iter_erase(&it);
		it = ax_box_begin(hmap_r.box);
	}

	axut_assert(r, ax_box_size(hmap_r.box) == 0);

	for (int i = 0; i < N; i++) {
		axut_assert(r, table[i] == 1);
	}
}

static void map_chkey(axut_runner* r)
{
	ax_base* base = axut_runner_arg(r);
	ax_hmap_r hmap_r = ax_hmap_create(ax_base_local(base), ax_stuff_traits(AX_ST_I),
			ax_stuff_traits(AX_ST_I));
	int k, v;

	k = 1, v = 2;
	if (!ax_map_put(hmap_r.map, &k, &v))
		axut_term(r, "ax_map_put");

	k = 3, v = 4;
	if (!ax_map_put(hmap_r.map, &k, &v))
		axut_term(r, "ax_map_put");

	k = 1;
	int new = 5;
	int *kp = ax_map_chkey(hmap_r.map, &k, &new);
	axut_assert_int_equal(r, 2, ax_box_size(hmap_r.box));
	axut_assert(r, kp != NULL);
	axut_assert_int_equal(r, new, *kp);

	axut_assert_int_equal(r, 2, *(int *)ax_map_get(hmap_r.map, &new));
}



static void iterate(axut_runner *r)
{
	ax_base* base = axut_runner_arg(r);
	ax_avl_r avl_r = ax_avl_create(ax_base_local(base), ax_stuff_traits(AX_ST_S),
			ax_stuff_traits(AX_ST_I32));
	const int count = 100;
	for (int32_t i = 0; i < count; i++) {
		char key[4];
		sprintf(key, "%d", i);
		ax_map_put(avl_r.map, key, &i);
	}

	int32_t check_table[count];
	for (int i = 0; i < count; i++) check_table[i] = -1;
	ax_map_cforeach(avl_r.map, const char *, key, const uint32_t *, val) {
		int k;
		sscanf(key, "%d", &k);
		axut_assert_int_equal(r, k, *val);
		check_table[k] = 0;
	}
	for (int i = 0; i < count; i++) 
		axut_assert(r, check_table[i] == 0);
}

static void clean(axut_runner *r)
{
	ax_base_destroy(axut_runner_arg(r));
}

axut_suite *suite_for_hmap(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "hmap");

	ax_base *base1 = ax_base_create();
	if (!base1)
		return NULL;
	axut_suite_set_arg(suite, base1);

	axut_suite_add(suite, iterate, 0);
	axut_suite_add(suite, map_erase, 1);
	axut_suite_add(suite, iter_erase, 1);
	axut_suite_add(suite, map_chkey, 1);
	axut_suite_add(suite, clean, 0xFF);
	return suite;
}
