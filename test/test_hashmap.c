#include <axut.h>

#include <axe/hashmap.h>
#include <axe.h>

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static void test_complex(axut_runner* r)
{
#define N 500
	ax_base* base = ax_base_create();
	ax_hashmap_role role = ax_hashmap_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));
	for (int k = 0, v = 0; k < N; k++, v++) {
		ax_map_put(role.map, &k, &v);
	}

	int table[N] = {0};
	ax_iter it = ax_box_begin(role.box), end = ax_box_end(role.box);
	while (!ax_iter_equal(it, end)) {
		ax_pair *pair = ax_iter_get(it);
		int *k = (int*)ax_pair_key(pair);
		int *v = (int*)ax_pair_value(pair);
		axut_assert(r, *k == *v);
		table[*k] = 1;
		it = ax_iter_next(it);
	}

	for (int i = 0; i < N; i++) {
		ax_map_erase(role.map, &i);
	}
	axut_assert(r, ax_box_size(role.box) == 0);

	for (int i = 0; i < N; i++) {
		axut_assert(r, table[i] == 1);
	}
	ax_base_destroy(base);
}

static void test_foreach(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_hashmap_role role = ax_hashmap_create(ax_base_local(base), ax_stuff_traits(AX_ST_S),
			ax_stuff_traits(AX_ST_I32));
	for (int32_t i = 0; i < 50; i++) {
		char key[4];
		sprintf(key, "%d", i);
		ax_map_put(role.map, key, &i);
	}

	int32_t check_table[50];
	for (int i = 0; i < 50; i++) check_table[i] = 50;
	ax_foreach(ax_pair *, pair, role.box) {
		int key;
		sscanf((char*)ax_pair_key(pair), "%d", &key);
		axut_assert(r, key == *(uint32_t*)ax_pair_value(pair));
		check_table[key] = 0;
	}
	for (int i = 0; i < 50; i++) 
		axut_assert(r, check_table[i] == 0);

	ax_base_destroy(base);
}

axut_suite *suite_for_hashmap(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "hashmap");
	axut_suite_add(suite, test_complex, 1);
	axut_suite_add(suite, test_foreach, 1);
	return suite;
}
