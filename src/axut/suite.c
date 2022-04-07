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

#include "axut/suite.h"
#include "axut/case.h"

#include "ax/vector.h"
#include "ax/algo.h"
#include "ax/mem.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "check.h"

struct axut_suite_st
{
	char *name;
	void *arg;
	ax_vector_r tctab;
};

void axut_suite_destroy(axut_suite *s)
{
	if (!s)
		return;
	ax_one_free(s->tctab.one);
	free(s->name);
	free(s);
}

axut_suite *axut_suite_create(const char *name)
{
	CHECK_PARAM_NULL(name);
	axut_suite *suite = NULL;
	ax_vector_r cases = ax_null;
	char *name_copy = NULL;
	
	suite = malloc(sizeof(axut_suite));
	if (!suite)
		return NULL;

	cases = ax_new(vector, &axut_case_tr);
	if (!cases.one) {
		return NULL;
	}

	name_copy = ax_strdup(name);
	if (!name_copy)
		goto fail;

	axut_suite suite_init = {
		.tctab = cases,
		.name = name_copy
	};
	memcpy(suite, &suite_init, sizeof suite_init);
	return suite;
fail:
	free(suite);
	ax_one_free(cases.one);
	free(name_copy);
	return NULL;
}

void axut_suite_set_arg(axut_suite *s, void *arg)
{
	s->arg = arg;
}

void *axut_suite_arg(const axut_suite *s)
{
	return s->arg;
}

ax_fail axut_suite_add_case(axut_suite *suite, const char *name, axut_case_proc_f proc, int priority)
{
	axut_case case_init  = {
		.name = (char*)name,
		.log = NULL,
		.proc = proc,
		.priority = priority,
		.state = AXUT_CS_READY
	};
	ax_fail fail = ax_seq_push(suite->tctab.seq, &case_init);
	if (fail)
		return fail;
	ax_iter first = ax_box_begin(suite->tctab.box);
	ax_iter last = ax_box_end(suite->tctab.box);
	ax_insertion_sort(&first, &last);
	return false;
}

const ax_seq *axut_suite_all_case(const axut_suite *suite)
{
	return suite->tctab.seq;
}

const char *axut_suite_name(const axut_suite *suite)
{
	return suite->name;
}
