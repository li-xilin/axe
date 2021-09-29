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

#include "axut/runner.h"
#include "axut/suite.h"

#include <ax/string.h>
#include <ax/vector.h>
#include <ax/mem.h>
#include <ax/avl.h>
#include <ax/seq.h>

#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <inttypes.h>

#include "check.h"

struct axut_runner_st
{
	ax_one _one;
	char *name;
	axut_output_f output_cb;
	ax_avl_r smap; // for removing suite
	ax_vector_r suites;
	ax_string_r output;
	struct {
		int pass;
		int fail;
		int term;

	} statistic;
	jmp_buf *jump_ptr;
	axut_case *current;
	void *arg;
};

void axut_runner_destroy(axut_runner *r)
{
	if (!r)
		return;
	ax_one_free(r->smap.one);
	ax_one_free(r->output.one);
	ax_box_foreach(r->suites.box, axut_suite **, ptr) {
		axut_suite_destroy(*ptr);
	}
	ax_one_free(r->suites.one);
	free(r);
}

static void default_output(const char *suite_name, axut_case *tc, ax_str *out)
{
	assert(tc->state != AXUT_CS_READY);
	switch (tc->state) {
		case AXUT_CS_PASS:
			ax_str_sprintf(out, "[ OK ] %-10s : %s\n", suite_name, tc->name);
			break;
		case AXUT_CS_FAIL:
			ax_str_sprintf(out, "[FAIL] %-10s : %s: %s, line %d: %s\n",
				suite_name, tc->name, tc->file, tc->line, tc->log ? tc->log : "none");
			break;
		case AXUT_CS_TERM:
			ax_str_sprintf(out, "[TERM] %-10s : %s: %s, line %d: %s\n",
				suite_name, tc->name, tc->file, tc->line, tc->log ? tc->log : "none");
			break;
	}
}

axut_runner *axut_runner_create(axut_output_f output_cb)
{
	axut_runner *runner = NULL;
	ax_avl_r smap = ax_null;
	ax_vector_r suites = ax_null;
	ax_string_r output = ax_null;

	runner = malloc(sizeof(axut_runner));
	if (!runner)
		goto fail;

	smap = ax_class_new(avl, ax_tr("p"), ax_tr("p"));
	if (!smap.one)
		goto fail;

	suites = ax_class_new(vector, ax_tr("p"));
	if (!suites.one)
		goto fail;

	output = ax_class_new0(string);
	if (!output.one)
		goto fail;

	axut_runner runner_init = {
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

	return runner;
fail:
	free(runner);
	ax_one_free(suites.one);
	ax_one_free(smap.one);
	ax_one_free(output.one);
	return NULL;
}

const char *axut_runner_result(const axut_runner *r)
{
	return ax_str_strz(r->output.str);
}

int axut_runner_summary(const axut_runner *r, int *pass, int *term)
{
	if (pass)
		*pass = r->statistic.pass;
	if (term)
		*term = r->statistic.term;
	return r->statistic.fail;
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
	ax_iter_prev(&last);

	if (!ax_map_put(r->smap.map, &s, &last.point)) {
		ax_seq_pop(r->suites.seq);
		return true;
	}

	return false;
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
	ax_iter_erase(&last);
}

void axut_runner_run(axut_runner *r)
{
	int case_count = 0, case_pass = 0;
	axut_output_f output_cb = r->output_cb ? r->output_cb : default_output;
	ax_box_clear(r->output.box);
	r->statistic.pass = 0;
	r->statistic.fail = 0;
	r->statistic.term = 0;
	ax_box_foreach(r->suites.box, axut_suite * const*, ppsuite) {
		const ax_seq *case_tab = axut_suite_all_case(*ppsuite);
		r->arg = axut_suite_arg(*ppsuite);
		ax_box_cforeach(ax_cr(seq, case_tab).box, const axut_case *, test_case) {
			jmp_buf jmp;
			axut_case *tc = (axut_case *)test_case;
			if (tc->state != AXUT_CS_READY)
				continue;
			r->jump_ptr = &jmp;
			r->current = tc;
			int jmpid = setjmp(jmp);
			switch (jmpid) {
				case 0:
					tc->proc(r);
					tc->state = AXUT_CS_PASS;
					case_pass ++;
					break;
				case 1:
					tc->state = AXUT_CS_FAIL;
					break;
				case 2:
					tc->state = AXUT_CS_TERM;
					break;
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

static void leave(axut_runner *r, axut_case_state cs, const char *file, int line, const char *fmt, va_list args)
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

	assert(cs == AXUT_CS_FAIL || cs == AXUT_CS_TERM);
	if (cs == AXUT_CS_FAIL)
		longjmp(*r->jump_ptr, 1);
	else
		longjmp(*r->jump_ptr, 2);
}

void __axut_assert(axut_runner *r, bool cond, const char *file, int line, const char *fmt, ...)
{
	if (cond)
		return;
	va_list args;
	va_start(args, fmt);
	leave(r, AXUT_CS_FAIL, file, line, fmt, args);
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
	leave(r, AXUT_CS_FAIL, file, line, fmt, args);
	va_end(args);
}

void __axut_term(axut_runner *r, const char *file, int line, const char *fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	leave(r, AXUT_CS_TERM, file, line, fmt, args);
	va_end(args);
}

