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

#include "ut/runner.h"
#include "ut/suite.h"
#include "ax/string.h"
#include "ax/trait.h"
#include "ax/vector.h"
#include "ax/mem.h"
#include "ax/avl.h"

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

#include "check.h"

ax_concrete_begin(ut_runner)
	char *name;
	ut_output_f output_cb;
	ax_avl_r smap; // for removing suite
	ax_vector_r suites;
	ax_string_r output;
	struct {
		int pass;
		int fail;
		int term;

	} statistic;
	jmp_buf *jump_ptr;
	ut_case *current;
	void *arg;
ax_end;

void one_free(ax_one *one)
{
	if (!one)
		return;
	ut_runner_r self = AX_R_INIT(ax_one, one);
	ax_one_free(self.ut_runner->smap.ax_one);
	ax_one_free(self.ut_runner->output.ax_one);
	ax_box_foreach(self.ut_runner->suites.ax_box, ut_suite **, ptr) {
		ut_suite_destroy(*ptr);
	}
	free(self.ax_one);
}

const char *one_name(const ax_one *one)
{
	return ax_class_name(1, ut_runner);
}

static void default_output(const char *suite_name, ut_case *tc, ax_str *out)
{
	assert(tc->state != UT_CS_READY);
	switch (tc->state) {
		case UT_CS_PASS:
			ax_str_sprintf(out, "[ OK ] %-10s : %s\n", suite_name, tc->name);
			break;
		case UT_CS_FAIL:
			ax_str_sprintf(out, "[FAIL] %-10s : %s: %s, line %d: %s\n",
				suite_name, tc->name, tc->file, tc->line, tc->log ? tc->log : "none");
			break;
		case UT_CS_TERM:
			ax_str_sprintf(out, "[TERM] %-10s : %s: %s, line %d: %s\n",
				suite_name, tc->name, tc->file, tc->line, tc->log ? tc->log : "none");
			break;
	}
}

const char *ut_runner_result(const ut_runner *r)
{
	return ax_str_strz(r->output.ax_str);
}

int ut_runner_summary(const ut_runner *r, int *pass, int *term)
{
	if (pass)
		*pass = r->statistic.pass;
	if (term)
		*term = r->statistic.term;
	return r->statistic.fail;
}

ax_fail ut_runner_add(ut_runner *r, ut_suite* s)
{
	CHECK_PARAM_NULL(r);
	CHECK_PARAM_NULL(s);

	ax_fail fail;
	fail = ax_seq_push(r->suites.ax_seq, &s);
	if (fail)
		return fail;

	ax_iter last = ax_box_end(r->suites.ax_box);
	ax_iter_prev(&last);

	if (!ax_map_put(r->smap.ax_map, &s, &last.point)) {
		ax_seq_pop(r->suites.ax_seq);
		return true;
	}

	return false;
}

void ut_runner_remove(ut_runner *r, ut_suite* s)
{
	CHECK_PARAM_NULL(r);
	CHECK_PARAM_NULL(s);
	ax_assert(ax_map_exist(r->smap.ax_map, &s), "the suite to remove does not exist");

	void **point = ax_map_get(r->smap.ax_map, &s);
	ax_iter last = ax_box_end(r->suites.ax_box);
	last.point = *point;
	ax_map_erase(r->smap.ax_map, &s);
	ax_iter_erase(&last);
}

void ut_runner_run(ut_runner *r)
{
	int case_count = 0, case_pass = 0;
	ut_output_f output_cb = r->output_cb ? r->output_cb : default_output;
	ax_box_clear(r->output.ax_box);
	r->statistic.pass = 0;
	r->statistic.fail = 0;
	r->statistic.term = 0;
	ax_box_foreach(r->suites.ax_box, ut_suite * const*, ppsuite) {
		const ax_seq *case_tab = ut_suite_all_case(*ppsuite);
		r->arg = ut_suite_arg(*ppsuite);
		ax_box_cforeach(ax_cr(ax_seq, case_tab).ax_box, const ut_case *, test_case) {
			jmp_buf jmp;
			ut_case *tc = (ut_case *)test_case;
			if (tc->state != UT_CS_READY)
				continue;
			r->jump_ptr = &jmp;
			r->current = tc;
			int jmpid = setjmp(jmp);
			switch (jmpid) {
				case 0:
					tc->proc(r);
					tc->state = UT_CS_PASS;
					case_pass ++;
					break;
				case 1:
					tc->state = UT_CS_FAIL;
					break;
				case 2:
					tc->state = UT_CS_TERM;
					break;
			}
			output_cb(ut_suite_name(*ppsuite), tc, r->output.ax_str);
			case_count ++;
		}
	}
	r->arg = NULL;
	ax_str_sprintf(r->output.ax_str, "PASS : %d / %d\n", case_pass, case_count);
}

void *ut_runner_arg(const ut_runner *r)
{
	return r->arg;
}

static void leave(ut_runner *r, ut_case_state cs, const char *file, int line, const char *fmt, va_list args)
{
	free(r->current->file);
	r->current->file = ax_strdup(file);

	r->current->line = line;

	free(r->current->log);
	r->current->log = NULL;
	if (fmt) {
		char buf[1024];
		vsprintf(buf, fmt, args);
		r->current->log =  ax_strdup(buf);
	}

	assert(cs == UT_CS_FAIL || cs == UT_CS_TERM);
	if (cs == UT_CS_FAIL)
		longjmp(*r->jump_ptr, 1);
	else
		longjmp(*r->jump_ptr, 2);
}

void __ut_assert(ut_runner *r, bool cond, const char *file, int line, const char *fmt, ...)
{
	if (cond)
		return;
	va_list args;
	va_start(args, fmt);
	leave(r, UT_CS_FAIL, file, line, fmt, args);
	va_end(args);
}

void __ut_assert_str_equal(ut_runner *r, const char *ex, const char *ac, const char *file, int line)
{
	if (strcmp(ex, ac) == 0)
		return;
	__ut_fail(r, file, line, "assertion failed: expect '%s', but actually '%s'", ex, ac);
}

void __ut_assert_int_equal(ut_runner *r, int64_t ex, int64_t ac, const char *file, int line)
{
	if (ex == ac)
		return;
	__ut_fail(r, file, line, "assertion failed: expect '%" PRId64 "', but actually '%" PRId64 "'", ex, ac);
}

void __ut_assert_uint_equal(ut_runner *r, uint64_t ex, uint64_t ac, const char *file, int line)
{
	if (ex == ac)
		return;
	__ut_fail(r, file, line, "assertion failed: expect '%" PRIu64 "', but actually '%" PRIu64 "'", ex, ac);
}

void __ut_fail(ut_runner *r, const char *file, int line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	leave(r, UT_CS_FAIL, file, line, fmt, args);
	va_end(args);
}

void __ut_term(ut_runner *r, const char *file, int line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	leave(r, UT_CS_TERM, file, line, fmt, args);
	va_end(args);
}

const ax_one_trait ut_runner_tr = {
	.name = one_name,
	.free = one_free,
};

ax_class_constructor(ut_runner, ut_output_f output_cb)
{
	ut_runner *runner = NULL;
	ax_avl_r smap = AX_R_NULL;
	ax_vector_r suites = AX_R_NULL;
	ax_string_r output = AX_R_NULL;

	runner = malloc(sizeof(ut_runner));
	if (!runner)
		goto fail;

	smap = ax_new(ax_avl, ax_t(ptr), ax_t(ptr));
	if (ax_r_isnull(smap))
		goto fail;

	suites = ax_new(ax_vector, ax_t(ptr));
	if (ax_r_isnull(suites))
		goto fail;

	output = ax_new0(ax_string);
	if (ax_r_isnull(output))
		goto fail;

	ut_runner runner_init = {
		.ax_one.tr = &ut_runner_tr,
		.statistic = {
			.pass = 0,
			.fail = 0,
			.term = 0
		},
		.smap = smap,
		.suites = suites,
		.output = output,
		.output_cb = output_cb ,
		.current = NULL,
		.arg = NULL
	};
	memcpy(runner, &runner_init, sizeof runner_init);

	return ax_r(ut_runner, runner).ax_one;
fail:
	free(runner);
	ax_one_free(suites.ax_one);
	ax_one_free(smap.ax_one);
	ax_one_free(output.ax_one);
	return NULL;
}

