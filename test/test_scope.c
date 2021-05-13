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

#include "ax/scope.h"
#include "ax/iter.h"
#include "ax/vector.h"
#include "ax/base.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(axut_runner *r)
{
	ax_base* base = axut_runner_arg(r);
	ax_scope_r role = ax_scope_create(ax_base_local(base));
	axut_assert(r, role.one != NULL);
	ax_scope_destroy(role.scope);
}

static void global(axut_runner *r)
{
	ax_base* base = axut_runner_arg(r);
	ax_vector_r role0 = ax_vector_create(ax_base_global(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_r role1 = ax_vector_create(ax_base_global(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_r role2 = ax_vector_create(ax_base_global(base), ax_stuff_traits(AX_ST_I32));
}

static void local(axut_runner *r)
{
	ax_base* base = axut_runner_arg(r);
	assert(base);
	ax_vector_r v1_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	{
		int d = ax_base_enter(base);
		ax_vector_r v2_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
		{
			int d = ax_base_enter(base);
			ax_vector_r v3_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
			ax_base_leave(base, d);
		}

		ax_base_leave(base, d);
	}
	ax_vector_r v4_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_r v5_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
}

static void cleanup(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_base_destroy(base);
}

axut_suite *suite_for_scope(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "scope");
	axut_suite_set_arg(suite, ax_base_create());

	axut_suite_add(suite, create, 0);
	axut_suite_add(suite, global, 0);
	axut_suite_add(suite, local, 0);
	axut_suite_add(suite, cleanup, 0xFF);

	return suite;
}
