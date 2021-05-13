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
#include "ax/base.h"

#include "axut.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static void init(axut_runner *r)
{
	ax_base* base = axut_runner_arg(r);

	int32_t table[] = {1, 2, 3, 4, 5};
	ax_vector_r vec_r;
	ax_list_r list_r;

	vec_r = ax_vector_init(ax_base_local(base), "i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof table));

	vec_r = ax_vector_init(ax_base_local(base), "i32x1", 1);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof *table));

	vec_r = ax_vector_init(ax_base_local(base), "i32_i32_i32_i32_i32", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof table));

	list_r = ax_list_init(ax_base_local(base), "i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof table));

	list_r = ax_list_init(ax_base_local(base), "&i32", table, 5);
	axut_assert(r, seq_equal_array(vec_r.seq, table, sizeof table));
}

static void pushl(axut_runner *r)
{
	ax_base* base = axut_runner_arg(r);

	int32_t table[] = {1, 2, 3, 4, 5};
	ax_list_r list;
	ax_vector_r vector;
	list = ax_list_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_seq_pushl(list.seq, "i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(list.seq, table, sizeof table));

	vector = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_seq_pushl(vector.seq, "i32x5", 1, 2, 3, 4, 5);
	axut_assert(r, seq_equal_array(vector.seq, table, sizeof table));
}

static void clean(axut_runner *r)
{
	ax_base* base = axut_runner_arg(r);
	ax_base_destroy(base);
}

axut_suite *suite_for_seq(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "seq");

	ax_base* base1 = ax_base_create();
	if (!base1)
		return NULL;

	axut_suite_set_arg(suite, base1);

	axut_suite_add(suite, init, 0);
	axut_suite_add(suite, pushl, 0);
	axut_suite_add(suite, clean, 0xFF);

	return suite;
}
