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

#ifndef AXUT_RUNNER_H
#define AXUT_RUNNER_H

#include "case.h"

#include "../ax/def.h"
#include "../ax/type/str.h"

#ifndef AX_RUNNER_DEFINED
#define AX_RUNNER_DEFINED
typedef struct ax_runner_st ax_runner;
#endif

#ifndef AXUT_SUITE_DEFINED
#define AXUT_SUITE_DEFINED
typedef struct axut_suite_st axut_suite;
#endif

#ifndef AXUT_CASE_DEFINED
#define AXUT_CASE_DEFINED
typedef struct axut_case_st axut_case;
#endif

#define ax_baseof_runner one
ax_concrete(1, runner);

typedef void (*axut_output_f)(const char* suite_name, axut_case *tc, ax_str *out);

const char *axut_runner_result(const ax_runner *r);

int axut_runner_summary(const ax_runner *r, int *pass, int *term);

ax_fail axut_runner_add(ax_runner *r, axut_suite* s);

void axut_runner_remove(ax_runner *r, axut_suite* s);

void axut_runner_run(ax_runner *r);

void *axut_runner_arg(const ax_runner *r);

void __axut_assert(ax_runner *r, bool cond, const char *file, int line, const char *fmt, ...);

void __axut_assert_str_equal(ax_runner *r, const char *ex, const char *ac, const char *file, int line);

void __axut_assert_int_equal(ax_runner *r, int64_t ex, int64_t ac, const char *file, int line);

void __axut_assert_uint_equal(ax_runner *r, uint64_t ex, uint64_t ac, const char *file, int line);

void __axut_fail(ax_runner *r, const char *file, int line, const char *fmt, ...);

void __axut_term(ax_runner *r, const char *file, int line, const char *fmt, ...);

#define axut_assert(r, cond) __axut_assert((r), (cond), __FILE__, __LINE__, "assertion failed: %s", #cond)

#define axut_assert_msg(r, cond, ...) __axut_assert((r), (cond), __FILE__, __LINE__, __VA_ARGS__)

#define axut_fail(r, ...) __axut_fail((r), __FILE__, __LINE__, __VA_ARGS__)

#define axut_term(r, ...) __axut_term((r), __FILE__, __LINE__, __VA_ARGS__)

#define axut_assert_str_equal(r, ex, ac) __axut_assert_str_equal((r), ex, ac, __FILE__, __LINE__)

#define axut_assert_int_equal(r, ex, ac) __axut_assert_int_equal((r), ex, ac, __FILE__, __LINE__)

#define axut_assert_uint_equal(r, ex, ac) __axut_assert_uint_equal((r), ex, ac, __FILE__, __LINE__)

ax_class_constructor(runner, axut_output_f ran_cb);

#endif

