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

#ifndef UT_RUNNER_H
#define UT_RUNNER_H

#include "case.h"

#include "../ax/def.h"
#include "../ax/type/str.h"

#ifndef UT_RUNNER_DEFINED
#define UT_RUNNER_DEFINED
typedef struct ut_runner_st ut_runner;
#endif

#ifndef UT_SUITE_DEFINED
#define UT_SUITE_DEFINED
typedef struct ut_suite_st ut_suite;
#endif

#ifndef UT_CASE_DEFINED
#define UT_CASE_DEFINED
typedef struct ut_case_st ut_case;
#endif

#define ax_baseof_ut_runner ax_one
ax_concrete_declare(1, ut_runner);

typedef void (*ut_output_f)(const char* suite_name, ut_case *tc, ax_str *out);

const char *ut_runner_result(const ut_runner *r);

int ut_runner_summary(const ut_runner *r, int *pass, int *term);

ax_fail ut_runner_add(ut_runner *r, ut_suite* s);

void ut_runner_remove(ut_runner *r, ut_suite* s);

void ut_runner_run(ut_runner *r);

void *ut_runner_arg(const ut_runner *r);

void __ut_assert(ut_runner *r, bool cond, const char *file, int line, const char *fmt, ...);

void __ut_assert_str_equal(ut_runner *r, const char *ex, const char *ac, const char *file, int line);

void __ut_assert_mem_equal(ut_runner *r, const void *ex, size_t exlen, const void *ac, size_t aclen, const char *file, int line);

void __ut_assert_int_equal(ut_runner *r, int64_t ex, int64_t ac, const char *file, int line);

void __ut_assert_uint_equal(ut_runner *r, uint64_t ex, uint64_t ac, const char *file, int line);

void __ut_fail(ut_runner *r, const char *file, int line, const char *fmt, ...);

void __ut_term(ut_runner *r, const char *file, int line, const char *fmt, ...);

#define ut_assert(r, cond) __ut_assert((r), (cond), __FILE__, __LINE__, "assertion failed: %s", #cond)

#define ut_assert_msg(r, cond, ...) __ut_assert((r), (cond), __FILE__, __LINE__, __VA_ARGS__)

#define ut_fail(r, ...) __ut_fail((r), __FILE__, __LINE__, __VA_ARGS__)

#define ut_term(r, ...) __ut_term((r), __FILE__, __LINE__, __VA_ARGS__)

#define ut_assert_str_equal(r, ex, ac) __ut_assert_str_equal((r), ex, ac, __FILE__, __LINE__)

#define ut_assert_mem_equal(r, ex, exlen, ac, aclen) __ut_assert_mem_equal((r), (ex), (exlen), (ac), (aclen), __FILE__, __LINE__)

#define ut_assert_int_equal(r, ex, ac) __ut_assert_int_equal((r), ex, ac, __FILE__, __LINE__)

#define ut_assert_uint_equal(r, ex, ac) __ut_assert_uint_equal((r), ex, ac, __FILE__, __LINE__)

ax_concrete_creator(ut_runner, ut_output_f ran_cb);

#endif

