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

#ifndef UT_SUITE_H
#define UT_SUITE_H

#include "case.h"
#include "../ax/type/str.h"
#include "../ax/type/box.h"

#define UT_NAME_MAX 128
#define UT_LOG_MAX 128

#ifndef UT_RUNNER_DEFINED
#define UT_RUNNER_DEFINED
typedef struct ut_runner_st ut_runner;
#endif

#ifndef UT_SUITE_DEFINED
#define UT_SUITE_DEFINED
typedef struct ut_suite_st ut_suite;
#endif

typedef enum ut_case_state_en
{
	UT_CS_READY = 0,
	UT_CS_PASS,
	UT_CS_FAIL,
	UT_CS_TERM,
} ut_case_state;

ut_suite *ut_suite_create(const char* name);

void ut_suite_destroy(ut_suite *s);

void ut_suite_set_arg(ut_suite *s, void *arg);

void *ut_suite_arg(const ut_suite *s);

ax_fail ut_suite_add_case(ut_suite *s, const char *name, ut_case_proc_f proc, int priority);

const ax_seq *ut_suite_all_case(const ut_suite *suite);

const char *ut_suite_name(const ut_suite *suite);

#define ut_suite_add(suite, proc, priority) ut_suite_add_case((suite), #proc, (proc), (priority))

#endif

