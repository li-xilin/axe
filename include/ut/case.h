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

#ifndef UT_CASE_H
#define UT_CASE_H

#include "ax/trait.h"
#include "ax/link.h"

#ifndef UT_CASE_DEFINED
#define UT_CASE_DEFINED
typedef struct ut_case_st ut_case;
#endif

#ifndef UT_RUNNER_DEFINED
#define UT_RUNNER_DEFINED
typedef struct ut_runner_st ut_runner;
#endif

typedef void (*ut_case_proc_f)(ut_runner *runner);

typedef enum ut_case_state_en
{
        UT_CS_READY = 0,
        UT_CS_PASS,
        UT_CS_FAIL,
        UT_CS_TERM,
} ut_case_state;

struct ut_case_st
{
	ut_case_proc_f proc;
	char *name;
	char *log;
	char *file;
	unsigned int line;
	int state;
	const int priority;
	ax_link msg_list;
};

extern const ax_trait ut_case_tr;

ax_fail ut_case_add_text(ut_case *c, const char *file, int line, char *msg);

typedef void ax_case_enum_text_f(ut_case *c, const char *file, int line, char *msg, void *ctx);
void ut_case_enum_text(ut_case *c, ax_case_enum_text_f *f, void *ctx);

#endif

