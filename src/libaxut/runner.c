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

#include <axut/runner.h>
#include <axut/suite.h>

#include <axe/base.h>
#include <axe/string.h>
#include <axe/vector.h>
#include <axe/mem.h>
#include <axe/avltree.h>
#include <axe/pair.h>
#include <axe/seq.h>
#include <axe/scope.h>
#include <axe/pool.h>
#include <axe/error.h>

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

#include "check.h"

struct axut_runner_st
{
	ax_one __one;
	ax_one_env one_env;
	char *name;
	axut_output_f output_cb;
	ax_avltree_role smap; // for removing suite
	ax_vector_role suites;
	ax_string_role  output;
	jmp_buf *jump_ptr;
	axut_case *current;
	void *arg;
};

static void one_free(ax_one *one);

static void one_free(ax_one *one)
{
	if (!one)
		return;

	axut_runner_role runner_rl = { .one = one };

	ax_scope_detach(one);
	ax_one_free(runner_rl.runner->smap.one);
	ax_one_free(runner_rl.runner->output.one);
	ax_one_free(runner_rl.runner->suites.one);
	ax_pool_free(runner_rl.runner);
}

static const ax_one_trait one_trait = {
	.name = "one.suite",
	.free = one_free,
	.envp = offsetof(axut_runner, one_env)
};


static void default_output(const char *suite_name, axut_case *tc, ax_str *out)
{
	assert(tc->state != AXUT_CS_READY);
	if (tc->state == AXUT_CS_PASS) {
		ax_str_sprintf(out, "[ OK ] %-10s : %s\n", suite_name, tc->name);
	} else {
		ax_str_sprintf(out, "[FAIL] %-10s : %s: %s, line %d: %s\n",
				suite_name, tc->name, tc->file, tc->line, tc->log);
	}
}

ax_one *__axut_runner_construct(ax_base *base, axut_output_f output_cb)
{
	CHECK_PARAM_NULL(base);

	ax_pool *pool = ax_base_pool(base);
	axut_runner *runner = NULL;
	ax_map *smap = NULL;
	ax_seq *suites = NULL;
	ax_str *output = NULL;

	runner = ax_pool_alloc(pool, sizeof(axut_runner));
	if (!runner) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	smap = __ax_avltree_construct(base, ax_stuff_traits(AX_ST_PTR), ax_stuff_traits(AX_ST_PTR));
	if (!smap)
		goto fail;

	suites = __ax_vector_construct(base, ax_stuff_traits(AX_ST_PTR));
	if (!suites)
		goto fail;

	output = __ax_string_construct(base);
	if (!output)
		goto fail;

	axut_runner runner_init = {
		.__one = {
			.base = base,
			.tr = &one_trait
		},
		.one_env = {
			.scope = NULL,
			.sindex = 0,
		},
		.smap = {
			.map = smap 
		},
		.suites = {
			.seq = suites
		},
		.output = { .str = output },
		.output_cb = output_cb ,
		.current = NULL,
		.arg = NULL
			
	};
	memcpy(runner, &runner_init, sizeof runner_init);

	return axut_cast(runner, runner).one;
fail:
	ax_pool_free(runner);
	ax_one_free(ax_cast(seq, suites).one);
	ax_one_free(ax_cast(map, smap).one);
	ax_one_free(ax_cast(str, output).one);
	return NULL;
}

axut_runner *axut_runner_create(ax_scope *scope, axut_output_f ran_cb)
{
	CHECK_PARAM_NULL(scope);

	ax_base *base = ax_one_base(ax_cast(scope, scope).one);
	axut_runner_role runner_rl = { .one = __axut_runner_construct(base, ran_cb) };
	if (runner_rl.one == NULL)
		return runner_rl.runner;
	ax_scope_attach(scope, runner_rl.one);
	return runner_rl.runner;

}

const char *axut_runner_result(const axut_runner *r)
{
	return ax_str_cstr(r->output.str);
}

ax_fail axut_runner_add(axut_runner *r, axut_suite* s)
{
	CHECK_PARAM_NULL(r);
	CHECK_PARAM_NULL(s);

	ax_fail fail;
	fail = ax_seq_push(r->suites.seq, &s);
	if (fail)
		return fail;

	ax_iter last = ax_box_end(r->suites.box);
	last = ax_iter_prev(last);

	fail = ax_map_put(r->smap.map, &s, &last.point);
	if (fail) {
		ax_seq_pop(r->suites.seq);
		return fail;
	}

	return ax_false;
}

void axut_runner_remove(axut_runner *r, axut_suite* s)
{
	CHECK_PARAM_NULL(r);
	CHECK_PARAM_NULL(s);
	ax_assert(ax_map_exist(r->smap.map, &s), "the suite to remove does not exist");

	void **point = ax_map_get(r->smap.map, &s);
	ax_iter last = ax_box_end(r->suites.box);
	last.point = *point;
	ax_map_erase(r->smap.map, &s);
	ax_iter_erase(last);
}

void axut_runner_run(axut_runner *r)
{
	int case_count = 0, case_pass = 0;
	axut_output_f output_cb = r->output_cb ? r->output_cb : default_output;
	ax_foreach(axut_suite * const*, ppsuite, r->suites.box) {
		const ax_seq *case_tab = axut_suite_all_case(*ppsuite);
		r->arg = axut_suite_arg(*ppsuite);
		ax_foreach(axut_case *, tc, &case_tab->__box) {
			jmp_buf jmp;
			if (tc->state != AXUT_CS_READY)
				continue;
			r->jump_ptr = &jmp;
			r->current = tc;
			if(setjmp(jmp) == 0) {
				tc->proc(r);
				tc->state = AXUT_CS_PASS;
				case_pass ++;
			} else {
				tc->state = AXUT_CS_FAIL;
			}
			output_cb(axut_suite_name(*ppsuite), tc, r->output.str);
			case_count ++;
		}
	}
	r->arg = NULL;
	ax_str_sprintf(r->output.str, "PASS : %d / %d\n", case_pass, case_count);
}

void *axut_runner_arg(const axut_runner *r)
{
	return r->arg;
}

static void fail(axut_runner *r, const char *file, int line, const char *fmt, va_list args)
{
	char buf[1024];
	ax_base *base = ax_one_base(axut_cast(runner, r).one);
	ax_pool *pool = ax_base_pool(base);

	ax_pool_free(r->current->file);
	r->current->file = ax_strdup(pool, file);

	r->current->line = line;

	ax_pool_free(r->current->log);
	vsprintf(buf, fmt, args);
	r->current->log =  ax_strdup(pool, buf);
	longjmp(*r->jump_ptr, 1);
}

void __axut_assert(axut_runner *r, ax_bool cond, const char *file, int line, const char *fmt, ...)
{
	if (cond)
		return;
	va_list args;
	va_start(args, fmt);
	fail(r, file, line, fmt, args);
	va_end(args);
}

void __axut_assert_str_equal(axut_runner *r, const char *ex, const char *ac, const char *file, int line)
{
	if (strcmp(ex, ac) == 0)
		return;
	__axut_fail(r, file, line, "assertion failed: expect '%s', but actually '%s'", ex, ac);
}

void __axut_assert_int_equal(axut_runner *r, int64_t ex, int64_t ac, const char *file, int line)
{
	if (ex == ac)
		return;
	__axut_fail(r, file, line, "assertion failed: expect '%" PRId64 "', but actually '%" PRId64 "'", ex, ac);
}

void __axut_assert_uint_equal(axut_runner *r, uint64_t ex, uint64_t ac, const char *file, int line)
{
	if (ex == ac)
		return;
	__axut_fail(r, file, line, "assertion failed: expect '%" PRIu64 "', but actually '%" PRIu64 "'", ex, ac);
}

void __axut_fail(axut_runner *r, const char *file, int line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	fail(r, file, line, fmt, args);
	va_end(args);
}

