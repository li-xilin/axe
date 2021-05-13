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

#include <axut/suite.h>

#include <ax/one.h>
#include <ax/vector.h>
#include <ax/algo.h>
#include <ax/seq.h>
#include <ax/scope.h>
#include <ax/pool.h>
#include <ax/mem.h>

#include <stdarg.h>
#include <string.h>
#include <stdio.h>

#include "check.h"

struct axut_suite_st
{
	ax_one _one;
	ax_one_env one_env;
	char *name;
	void *arg;
	ax_vector_r tctab;
};

static void one_free(ax_one *one);

static void one_free(ax_one *one)
{
	CHECK_PARAM_NULL(one);

	axut_suite_role suite_rl = { .one = one };

	ax_scope_detach(one);
	ax_one_free(suite_rl.suite->tctab.one);
	ax_pool_free(suite_rl.suite->name);
	ax_pool_free(suite_rl.suite);
}

static const ax_one_trait one_trait = {
	.name = "one.suite",
	.free = one_free
};

static void case_free(void *p)
{
	axut_case *tc = p;
	ax_pool_free(tc->name);
	ax_pool_free(tc->file);
	ax_pool_free(tc->log);
}

static bool case_less(const void *p1, const void *p2, size_t size)
{

	const axut_case *tc1 = p1;
	const axut_case *tc2 = p2;
	return tc1->priority < tc2->priority;
}

static bool case_equal(const void *p1, const void *p2, size_t size)
{

	const axut_case *tc1 = p1;
	const axut_case *tc2 = p2;
	return tc1->priority == tc2->priority;
}

static ax_fail case_copy(ax_pool* pool, void* dst, const void* src, size_t size)
{
	const axut_case *src_tc = src;
	axut_case *dst_tc = dst;
	memcpy(dst_tc, src_tc, sizeof *dst_tc);
	dst_tc->name = dst_tc->log = dst_tc->file = NULL;
	dst_tc->name = ax_strdup(pool, src_tc->name);
	if (!src_tc->name)
		goto out;

	if (src_tc->log) {
		dst_tc->log =  ax_strdup(pool, src_tc->log);
		if (!dst_tc->log)
			goto out;

	}
	if (src_tc->file) {
		dst_tc->file =  ax_strdup(pool, src_tc->file);
		if (!dst_tc->file)
			goto out;

	}

	return false;
out:
	ax_pool_free(dst_tc->name);
	ax_pool_free(dst_tc->log);
	ax_pool_free(dst_tc->file);
	return true;
}

static const ax_stuff_trait case_trait = {
	.copy = case_copy,
	.move = ax_stuff_mem_move,
	.init = ax_stuff_mem_init,
	.swap = ax_stuff_mem_swap,
	.less = case_less,
	.equal = case_equal,
	.free = case_free,
	.size = sizeof(axut_case),
	.link = false

};

ax_one *__axut_suite_construct(ax_base *base, const char* name)
{
	CHECK_PARAM_NULL(base);

	ax_pool *pool = ax_base_pool(base);

	axut_suite *suite = ax_pool_alloc(pool, sizeof(axut_suite));
	if (suite == NULL)
		return NULL;

	ax_seq *tctab = __ax_vector_construct(base, &case_trait);
	if (tctab == NULL) {
		ax_pool_free(suite);
		return NULL;
	}

	char *name_copy = ax_strdup(pool, name);

	axut_suite suite_init = {
		._one = {
			.tr = &one_trait,
				.env = {
					.base = base,
					.scope = { NULL },
				},
		},
		.tctab = {
			.seq = tctab 
		},
		.name = name_copy
	};
	memcpy(suite, &suite_init, sizeof suite_init);
	return &suite->_one;
}

axut_suite *axut_suite_create(ax_scope *scope, const char *name)
{
	CHECK_PARAM_NULL(scope);
	ax_base *base = ax_one_base(ax_r(scope, scope).one);
	axut_suite_role suite_rl = { .one = __axut_suite_construct(base, name) };
	if (suite_rl.one == NULL)
		return suite_rl.suite;
	ax_scope_attach(scope, suite_rl.one);
	return suite_rl.suite;
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
	axut_case tc_init  = {
		.name = (char*)name,
		.log = NULL,
		.proc = proc,
		.priority = priority,
		.state = AXUT_CS_READY
	};
	ax_fail fail = ax_seq_push(suite->tctab.seq, &tc_init);
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
