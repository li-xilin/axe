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

#include "assist.h"
#include "ax/class.h"
#include "ax/list.h"
#include "ax/vector.h"
#include "ax/avl.h"
#include "ax/hmap.h"
#include "ax/stack.h"
#include "ax/queue.h"
#include "ax/btrie.h"
#include "ut/runner.h"
#include "ut/suite.h"

static void new(ut_runner *r)
{
	ax_list_r obj1 = ax_new(ax_list, ax_t(i8));
	ut_assert(r, obj1.ax_one != NULL);
	ut_assert(r, obj1.ax_seq->env.ax_box.elem_tr == ax_t(i8));

	ax_vector_r obj2 = ax_new(ax_vector, ax_t(u8));
	ut_assert(r, obj2.ax_one != NULL);
	ut_assert(r, obj2.ax_seq->env.ax_box.elem_tr == ax_t(u8));

	ax_avl_r obj3 = ax_new(ax_avl, ax_t(str), ax_t(i32));
	ut_assert(r, obj3.ax_one != NULL);
	ut_assert(r, obj3.ax_map->env.key_tr == ax_t(str));
	ut_assert(r, obj3.ax_map->env.ax_box.elem_tr == ax_t(i32));

	ax_hmap_r obj4 = ax_new(ax_hmap, ax_t(size), ax_t(u64));
	ut_assert(r, obj4.ax_one != NULL);
	ut_assert(r, obj4.ax_map->env.key_tr == ax_t(size));
	ut_assert(r, obj4.ax_map->env.ax_box.elem_tr == ax_t(u64));

	ax_stack_r obj5 = ax_new(ax_stack, ax_t(ptr));
	ut_assert(r, obj5.ax_one != NULL);
	ut_assert(r, obj5.ax_tube->env.elem_tr == ax_t(ptr));

	ax_queue_r obj6 = ax_new(ax_queue, ax_t(double));
	ut_assert(r, obj6.ax_one != NULL);
	ut_assert(r, obj6.ax_tube->env.elem_tr == ax_t(double));
	
	ax_btrie_r obj7 = ax_new(ax_btrie, ax_t(float), ax_t(wcs));
	ut_assert(r, obj7.ax_one != NULL);
	ut_assert(r, obj7.ax_trie->env.key_tr == ax_t(float));
	ut_assert(r, obj7.ax_trie->env.ax_box.elem_tr == ax_t(wcs));

	ax_one_free(obj1.ax_one);
	ax_one_free(obj2.ax_one);
	ax_one_free(obj3.ax_one);
	ax_one_free(obj4.ax_one);
	ax_one_free(obj5.ax_one);
	ax_one_free(obj6.ax_one);
	ax_one_free(obj7.ax_one);
}

ut_suite *suite_for_class()
{
	ut_suite* suite = ut_suite_create("class");
	ut_suite_add(suite, new, 0);
	return suite;
}
