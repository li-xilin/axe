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

#include "ut/suite.h"
#include "ut/case.h"

#include "ax/vector.h"
#include "ax/algo.h"
#include "ax/mem.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "check.h"

struct ut_suite_st
{
	char *name;
	void *arg;
	ax_vector_r tctab;
};

void ut_suite_destroy(ut_suite *s)
{
	if (!s)
		return;
	ax_one_free(s->tctab.ax_one);
	free(s->name);
	free(s);
}

ut_suite *ut_suite_create(const char *name)
{
	CHECK_PARAM_NULL(name);
	ut_suite *suite = NULL;
	ax_vector_r cases = AX_R_NULL;
	char *name_copy = NULL;
	
	suite = malloc(sizeof(ut_suite));
	if (!suite)
		return NULL;

	cases = ax_new(ax_vector, &ut_case_tr);
	if (ax_r_isnull(cases)) {
		return NULL;
	}

	name_copy = ax_strdup(name);
	if (!name_copy)
		goto fail;

	ut_suite suite_init = {
		.tctab = cases,
		.name = name_copy
	};
	memcpy(suite, &suite_init, sizeof suite_init);
	return suite;
fail:
	free(suite);
	ax_one_free(cases.ax_one);
	free(name_copy);
	return NULL;
}

void ut_suite_set_arg(ut_suite *s, void *arg)
{
	s->arg = arg;
}

void *ut_suite_arg(const ut_suite *s)
{
	return s->arg;
}

ax_fail ut_suite_add_case(ut_suite *suite, const char *name, ut_case_proc_f proc, int priority)
{
	ut_case case_init  = {
		.name = (char*)name,
		.log = NULL,
		.proc = proc,
		.priority = priority,
		.state = UT_CS_READY
	};
	ax_fail fail = ax_seq_push(suite->tctab.ax_seq, &case_init);
	if (fail)
		return fail;
	ax_iter first = ax_box_begin(suite->tctab.ax_box);
	ax_iter last = ax_box_end(suite->tctab.ax_box);
	ax_insertion_sort(&first, &last);
	return false;
}

const ax_seq *ut_suite_all_case(const ut_suite *suite)
{
	return suite->tctab.ax_seq;
}

const char *ut_suite_name(const ut_suite *suite)
{
	return suite->name;
}
