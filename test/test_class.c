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

#include "axut.h"

static void new(axut_runner *r)
{
	ax_list_r obj1 = ax_class_new(list, ax_t(i8));
	axut_assert(r, obj1.one != NULL);
	axut_assert(r, obj1.seq->env.box.elem_tr == ax_stuff_traits(AX_ST_I8));

	ax_vector_r obj2 = ax_class_new(vector, ax_t(u8));
	axut_assert(r, obj2.one != NULL);
	axut_assert(r, obj2.seq->env.box.elem_tr == ax_stuff_traits(AX_ST_U8));

	ax_avl_r obj3 = ax_class_new(avl, ax_t(str), ax_t(i32));
	axut_assert(r, obj3.one != NULL);
	axut_assert(r, obj3.map->env.key_tr == ax_stuff_traits(AX_ST_S));
	axut_assert(r, obj3.map->env.box.elem_tr == ax_stuff_traits(AX_ST_I32));

	ax_hmap_r obj4 = ax_class_new(hmap, ax_t(size), ax_t(u64));
	axut_assert(r, obj4.one != NULL);
	axut_assert(r, obj4.map->env.key_tr == ax_stuff_traits(AX_ST_Z));
	axut_assert(r, obj4.map->env.box.elem_tr == ax_stuff_traits(AX_ST_U64));

	ax_stack_r obj5 = ax_class_new(stack, ax_t(ptr));
	axut_assert(r, obj5.one != NULL);
	axut_assert(r, obj5.tube->env.elem_tr == ax_stuff_traits(AX_ST_PTR));

	ax_queue_r obj6 = ax_class_new(queue, ax_t(double));
	axut_assert(r, obj6.one != NULL);
	axut_assert(r, obj6.tube->env.elem_tr == ax_stuff_traits(AX_ST_LF));
	
	ax_btrie_r obj7 = ax_class_new(btrie, ax_t(float), ax_t(wcs));
	axut_assert(r, obj7.one != NULL);
	axut_assert(r, obj7.trie->env.key_tr == ax_stuff_traits(AX_ST_F));
	axut_assert(r, obj7.trie->env.box.elem_tr == ax_stuff_traits(AX_ST_WS));

	ax_one_free(obj1.one);
	ax_one_free(obj2.one);
	ax_one_free(obj3.one);
	ax_one_free(obj4.one);
	ax_one_free(obj5.one);
	ax_one_free(obj6.one);
	ax_one_free(obj7.one);
}

axut_suite *suite_for_class()
{
	axut_suite* suite = axut_suite_create("class");
	axut_suite_add(suite, new, 0);
	return suite;
}
