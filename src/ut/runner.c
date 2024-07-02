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

static void one_free(ax_one *one)
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

static const char *one_name(const ax_one *one)
{
	return ax_class_name(1, ut_runner);
}


static void case_enum(ut_case *c, const char *file, int line, char *msg, void *ctx)
{
	void **ctx_arr = (void **)ctx;
	ax_str *out = ctx_arr[0];
	const char *suite_name = ctx_arr[1];

	ax_str_sprintf(out, "[INFO] %-8s : %s:%s:%d: %s\n",
				suite_name, c->name, file, line, msg);
}

static void default_output(const char *suite_name, ut_case *tc, ax_str *out)
{
	assert(tc->state != UT_CS_READY);
	ut_case_enum_text(tc, case_enum, &(const void *[]) { out, suite_name });
	switch (tc->state) {
		case UT_CS_PASS:
			ax_str_sprintf(out, "[ OK ] %-8s : %s\n", suite_name, tc->name);
			break;
		case UT_CS_FAIL:
			ax_str_sprintf(out, "[FAIL] %-8s : %s:%s:%d: %s\n",
				suite_name, tc->name, tc->file, tc->line, tc->log ? tc->log : "none");
			break;
		case UT_CS_TERM:
			ax_str_sprintf(out, "[TERM] %-8s : %s:%s:%d: %s\n",
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

void ut_runner_run(ut_runner *r, ut_process_f *process_cb)
{
	int case_tested = 0, case_pass = 0;
	ut_output_f output_cb = r->output_cb ? r->output_cb : default_output;
	ax_box_clear(r->output.ax_box);
	r->statistic.pass = 0;
	r->statistic.fail = 0;
	r->statistic.term = 0;

	int case_total = 0;
	ax_box_foreach(r->suites.ax_box, ut_suite * const*, ppsuite)
		case_total += ut_suite_size(*ppsuite);

	ax_box_foreach(r->suites.ax_box, ut_suite * const*, ppsuite) {
		r->arg = ut_suite_arg(*ppsuite);
		size_t size = ut_suite_size(*ppsuite);
		for (size_t i = 0; i < size; i++) {
			jmp_buf jmp;
			ut_case *tc = ut_suite_at(*ppsuite, i);;
			if (tc->state != UT_CS_READY)
				continue;

			if (process_cb)
				process_cb(ut_suite_name(*ppsuite), tc->name, case_tested + 1, case_total);

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
			case_tested ++;
		}

	}
	r->arg = NULL;
	ax_str_sprintf(r->output.ax_str, "PASS : %d / %d\n", case_pass, case_tested);
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

void __ut_printf(ut_runner *r, const char *file, int line, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	char buf[1024];
	strcpy(buf, "failed to print log message.");
	vsnprintf(buf, sizeof buf, fmt, ap);
	ut_case_add_text(r->current, file, line, buf);
	va_end(ap);
}

void __ut_assert_str_equal(ut_runner *r, const char *ex, const char *ac, const char *file, int line)
{
	if (strcmp(ex, ac) == 0)
		return;
	__ut_fail(r, file, line, "test failed: expect '%s', but actual '%s'", ex, ac);
}

void __ut_assert_mem_equal(ut_runner *r, const void *ex, size_t exsize, const void *ac, size_t acsize, const char *file, int line)
{

	if (exsize != acsize) {
		__ut_fail(r, file, line, "test failed: expect size is %zd, but actual %zd", exsize, acsize);
		return;
	}

	int index = -1;
	int cmp = 0;

	const char *exp = ex, *acp = ac;
	for (int i = 0; i < exsize; i++) {
		if (exp[i] != acp[i]) {
			cmp = exp[i] - acp[i];
			index = i;
			break;
		}
	}

	if (cmp == 0)
		return;

	char ex_buf[66 + 1];
	char ac_buf[66 + 1];

	size_t left = ax_max(index - 16, 0);
	size_t right = ax_min((index + 1) + 16, exsize);

	ax_memtohex(exp + left, index - left, ex_buf);
	sprintf(ex_buf + (index - left) * 2, "[%hhX]", exp[index]);
	ax_memtohex(exp + index + 1, right - (index + 1), ex_buf + (index - left) * 2 + 4);

	ax_memtohex(acp + left, index - left, ac_buf);
	sprintf(ac_buf + (index - left) * 2, "[%hhX]", acp[index]);
	ax_memtohex(acp + index + 1, right - (index + 1), ac_buf + (index - left) * 2 + 4);



	__ut_fail(r, file, line, "test failed: at offset +%d\n"
			"\texpect: '%s'\n"
			"\tactual: '%s'", index, ex_buf, ac_buf);
}

void __ut_assert_int_equal(ut_runner *r, int64_t ex, int64_t ac, const char *file, int line)
{
	if (ex == ac)
		return;
	__ut_fail(r, file, line, "test failed: expect '%" PRId64 "', but actual '%" PRId64 "'", ex, ac);
}

void __ut_assert_uint_equal(ut_runner *r, uint64_t ex, uint64_t ac, const char *file, int line)
{
	if (ex == ac)
		return;
	__ut_fail(r, file, line, "test failed: expect '%" PRIu64 "', but actual '%" PRIu64 "'", ex, ac);
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

ax_concrete_creator(ut_runner, ut_output_f output_cb)
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

