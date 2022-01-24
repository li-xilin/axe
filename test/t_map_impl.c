/*
 * Copyright (c) 2022 Li hsilin <lihsilyn@gmail.com>
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
#include <ax/def.h>
#include "ax/map.h"
#include "axut.h"

#include "ax/avl.h"
#include "ax/hmap.h"
#include "ax/rb.h"
#include "ax/iter.h"
#include "axut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

typedef ax_map *create_map_f(void);

struct create_map_func {
	create_map_f *f;
};

static ax_map *create_empty_avl(void)
{
	return ax_class_new(avl, ax_t(int), ax_t(int)).map;
}

static ax_map *create_empty_hmap(void)
{
	return ax_class_new(hmap, ax_t(int), ax_t(int)).map;
}

static ax_map *create_empty_rb(void)
{
	return ax_class_new(rb, ax_t(int), ax_t(int)).map;
}

static void workflow(axut_runner *r)
{
	create_map_f *create = (create_map_f *)(intptr_t)axut_runner_arg(r);
	ax_map *map = create();
	ax_map_put(map, ax_p(int, 2), ax_p(int, 2));
	ax_map_put(map, ax_p(int, 1), ax_p(int, 1));
	ax_map_put(map, ax_p(int, 3), ax_p(int, 4));
	ax_map_put(map, ax_p(int, 3), ax_p(int, 3));
	axut_assert_int_equal(r, 3, ax_box_size(ax_r(map, map).box));

	axut_assert_int_equal(r, 1, *(int *) ax_map_get(map, ax_p(int, 1)));
	axut_assert_int_equal(r, 2, *(int *) ax_map_get(map, ax_p(int, 2)));
	axut_assert_int_equal(r, 3, *(int *) ax_map_get(map, ax_p(int, 3)));

	ax_map_erase(map, ax_p(int, 1));
	axut_assert(r, !ax_map_exist(map, ax_p(int, 0)));
	ax_map_erase(map, ax_p(int, 2));
	axut_assert(r, !ax_map_exist(map, ax_p(int, 1)));
	ax_map_erase(map, ax_p(int, 3));
	axut_assert(r, !ax_map_exist(map, ax_p(int, 2)));

	axut_assert_int_equal(r, 0, ax_box_size(ax_r(map, map).box));
	ax_one_free(ax_r(map, map).one);
}

static void shuffle(int *arr, size_t size)
{
	ax_repeat(size)
		arr[_] = _;
	
	ax_repeat(size)
		ax_swap(arr + _, arr + (rand() % size), int);
}

static void insert(axut_runner *r)
{
	create_map_f *create = (create_map_f *)(intptr_t)axut_runner_arg(r);
	ax_map *map = create();

	int nums[0x10];
	shuffle(nums, ax_nelems(nums));
	
	ax_repeat(ax_nelems(nums))
		ax_map_put(map, nums + _, nums + _);

	axut_assert_int_equal(r, ax_nelems(nums), ax_box_size(ax_r(map, map).box));

	ax_repeat(ax_nelems(nums)) {
		axut_assert(r, ax_map_exist(map, nums + _));
		axut_assert_int_equal(r, nums[_], *(int *)ax_map_get(map, nums + _));
	}

	ax_box_clear(ax_r(map, map).box);
	axut_assert_int_equal(r, 0, ax_box_size(ax_r(map, map).box));

	ax_repeat(ax_nelems(nums))
		ax_map_iput(map, nums + _, nums[_]);

	axut_assert_int_equal(r, ax_nelems(nums), ax_box_size(ax_r(map, map).box));

	ax_repeat(ax_nelems(nums))
		axut_assert(r, ax_map_exist(map, nums + _));

	ax_one_free(ax_r(map, map).one);
}

static void update(axut_runner *r)
{
	create_map_f *create = (create_map_f *)(intptr_t)axut_runner_arg(r);
	ax_map *map = create();

	int nums[0x800];
	shuffle(nums, ax_nelems(nums));

	ax_repeat(ax_nelems(nums))
		ax_map_put(map, nums + _, nums + _);

	ax_repeat(ax_nelems(nums))
		ax_map_put(map, nums + _, ax_p(int,  - *(int *)ax_map_get(map, nums + _)));

	axut_assert_int_equal(r, ax_nelems(nums), ax_box_size(ax_r(map, map).box));

	ax_map_foreach(map, int *, key, int *, val) {
		axut_assert_int_equal(r, - *key, *val);
		nums[*key] = -1;
	}
	ax_one_free(ax_r(map, map).one);
}

static void erase(axut_runner *r)
{
	create_map_f *create = (create_map_f *)(intptr_t)axut_runner_arg(r);
	ax_map *map = create();

	int nums[0x800];
	shuffle(nums, ax_nelems(nums));

	ax_repeat(ax_nelems(nums))
		ax_map_put(map, nums + _, nums + _);

	ax_repeat(ax_nelems(nums)) {
		ax_map_erase(map, nums + _);
		axut_assert(r, !ax_map_exist(map, nums + _));
	}
	axut_assert_int_equal(r, 0, ax_box_size(ax_r(map, map).box));

	ax_box_clear(ax_r(map, map).box);

	ax_repeat(ax_nelems(nums))
		ax_map_put(map, nums + _, nums + _);

	for (ax_iter it = ax_box_begin(ax_r(map, map).box);
			!ax_box_iter_ended(ax_r(map, map).box, &it);) {
		nums[*(int *)ax_iter_get(&it)] = -1;
		ax_iter_erase(&it);
	}
	axut_assert_int_equal(r, 0, ax_box_size(ax_r(map, map).box));

	ax_repeat(ax_nelems(nums))
		axut_assert_int_equal(r, -1, nums[_]);

	ax_one_free(ax_r(map, map).one);
}

static void iterator(axut_runner *r)
{
	create_map_f *create = (create_map_f *)(intptr_t)axut_runner_arg(r);
	ax_map *map = create();

	int nums[0x800];
	shuffle(nums, ax_nelems(nums));

	ax_repeat(ax_nelems(nums))
		ax_map_put(map, nums + _, nums + _);

	ax_box_iterate(ax_r(map, map).box, it) {
		int *val = ax_iter_get(&it);
		int *key = ax_map_iter_key(&it);
		axut_assert_int_equal(r, *val, *key);
	}

	ax_one_free(ax_r(map, map).one);
}

static void copy(axut_runner *r)
{
	create_map_f *create = (create_map_f *)(intptr_t)axut_runner_arg(r);
	ax_map *map = create();

	int nums[0x800];
	shuffle(nums, ax_nelems(nums));

	ax_repeat(ax_nelems(nums))
		ax_map_put(map, nums + _, nums + _);

	ax_map *map1 = (ax_map *)ax_any_copy(ax_r(map, map).any);

	ax_map_foreach(map1, int *, key, int *, val) {
		axut_assert(r, nums[*key] != -1);
		nums[*key] = -1;
		axut_assert_int_equal(r, *key, *val);
	}
	ax_one_free(ax_r(map,map).one);
	ax_one_free(ax_r(map,map1).one);
}

void suite_for_maps(axut_runner *r)
{
	for (int i = 0; i < 3; i++) {
		axut_suite *suite = NULL;
		switch(i) {
			case 0:
				suite = axut_suite_create("avl");
				axut_suite_set_arg(suite, (void *)(intptr_t)create_empty_avl);
				break;
			case 1:
				suite = axut_suite_create("hmap");
				axut_suite_set_arg(suite, (void *)(intptr_t)create_empty_hmap);
				break;
			case 2:
				suite = axut_suite_create("rb");
				axut_suite_set_arg(suite, (void *)(intptr_t)create_empty_rb);
				break;
		}

		axut_suite_add(suite, workflow, 0);
		axut_suite_add(suite, insert, 1);
		axut_suite_add(suite, update, 2);
		axut_suite_add(suite, iterator, 3);
		axut_suite_add(suite, erase, 4);
		axut_suite_add(suite, copy, 4);
		axut_runner_add(r, suite);
	}
}
