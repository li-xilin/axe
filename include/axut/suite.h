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

#ifndef AXUT_SUITE_H
#define AXUT_SUITE_H

#include "case.h"
#include "../ax/type/str.h"
#include "../ax/type/box.h"

#define AXUT_NAME_MAX 128
#define AXUT_LOG_MAX 128

#ifndef AX_RUNNER_DEFINED
#define AX_RUNNER_DEFINED
typedef struct ax_runner_st ax_runner;
#endif

#ifndef AXUT_SUITE_DEFINED
#define AXUT_SUITE_DEFINED
typedef struct axut_suite_st axut_suite;
#endif

typedef enum axut_case_state_en
{
	AXUT_CS_READY = 0,
	AXUT_CS_PASS,
	AXUT_CS_FAIL,
	AXUT_CS_TERM,
} axut_case_state;

axut_suite *axut_suite_create(const char* name);

void axut_suite_destroy(axut_suite *s);

void axut_suite_set_arg(axut_suite *s, void *arg);

void *axut_suite_arg(const axut_suite *s);

ax_fail axut_suite_add_case(axut_suite *s, const char *name, axut_case_proc_f proc, int priority);

const ax_seq *axut_suite_all_case(const axut_suite *suite);

const char *axut_suite_name(const axut_suite *suite);

#define axut_suite_add(suite, proc, priority) axut_suite_add_case((suite), #proc, (proc), (priority))

#endif

