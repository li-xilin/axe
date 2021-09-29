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
#include "ax/iter.h"
#include "ax/vector.h"
#include "ax/list.h"
#include "ax/algo.h"

#include "axut.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static void init(axut_runner *r)
{
	int32_t table[] = {1, 2, 3, 4, 5};
	ax_vector_r vec_r;
	ax_list_r list_r;

	vec_r = ax_vector_init("i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof table));
	ax_one_free(vec_r.one);

	vec_r = ax_vector_init("i32x1", 1);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof *table));
	ax_one_free(vec_r.one);

	vec_r = ax_vector_init("i32_i32_i32_i32_i32", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof table));
	ax_one_free(vec_r.one);

	list_r = ax_list_init("i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(list_r.seq, table, sizeof table));
	ax_one_free(list_r.one);

	list_r = ax_list_init("&i32", table, 5);
	axut_assert(r, seq_equal_array(list_r.seq, table, sizeof table));
	ax_one_free(list_r.one);
}

static void pushl(axut_runner *r)
{
	int32_t table[] = {1, 2, 3, 4, 5};
	ax_list_r list = ax_class_new(list, ax_stuff_traits(AX_ST_I32));
	ax_seq_pushl(list.seq, "i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(list.seq, table, sizeof table));

	ax_vector_r vector = ax_class_new(vector, ax_stuff_traits(AX_ST_I32));
	ax_seq_pushl(vector.seq, "i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(vector.seq, table, sizeof table));

	ax_one_free(list.one);
	ax_one_free(vector.one);
}

axut_suite *suite_for_seq()
{
	axut_suite* suite = axut_suite_create("seq");

	axut_suite_add(suite, init, 0);
	axut_suite_add(suite, pushl, 0);
	return suite;
}
