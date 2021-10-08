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

#ifndef AXUT_CASE_H
#define AXUT_CASE_H

#include "../ax/stuff.h"

#ifndef AXUT_CASE_DEFINED
#define AXUT_CASE_DEFINED
typedef struct axut_case_st axut_case;
#endif

#ifndef AXUT_RUNNER_DEFINED
#define AXUT_RUNNER_DEFINED
typedef struct axut_runner_st axut_runner;
#endif

typedef void (*axut_case_proc_f)(axut_runner *runner);

struct axut_case_st
{
	axut_case_proc_f proc;
	char *name;
	char *log;
	char *file;
	unsigned int line;
	int state;
	const int priority;
};

const ax_stuff_trait axut_case_tr;

#endif
