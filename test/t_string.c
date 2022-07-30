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

#include "ax/iter.h"
#include "ax/string.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(ut_runner *r)
{
	ax_string_r str_r = ax_new0(ax_string);
	ut_assert(r, str_r.ax_any != NULL);
	ut_assert(r, ax_box_size(str_r.ax_box) == 0);
	ut_assert(r, ax_str_length(str_r.ax_str) == 0);
	ax_one_free(str_r.ax_one);
}

static void append(ut_runner *r)
{
	ax_string_r str_r = ax_new0(ax_string);
	ax_str_append(str_r.ax_str, "hello");
	ax_str_append(str_r.ax_str, " world");
	ut_assert(r, strcmp(ax_str_strz(str_r.ax_str), "hello world") == 0);
	ut_assert(r, ax_str_length(str_r.ax_str) == sizeof "hello world" - 1);
	ax_str_insert(str_r.ax_str, 6, "my ");
	ut_assert(r, strcmp(ax_str_strz(str_r.ax_str), "hello my world") == 0);
	ax_one_free(str_r.ax_one);
}

static void split(ut_runner *r)
{
	ax_string_r str_r = ax_new0(ax_string);
	ax_str_append(str_r.ax_str, ":111:222::");
	ax_seq_r ret =  ax_str_split(str_r.ax_str, ':');

	int i = 0;
	ax_box_cforeach(ret.ax_box, const char*, p) {
		switch(i) {
			case 0: ut_assert_str_equal(r, "", p); break;
			case 1: ut_assert_str_equal(r, "111", p); break;
			case 2: ut_assert_str_equal(r, "222", p); break;
			case 3: ut_assert_str_equal(r, "", p); break;
			case 4: ut_assert_str_equal(r, "", p); break;
		}
		i++;
	}
	
	ax_one_free(str_r.ax_one);
	ax_one_free(ret.ax_one);
}

ut_suite *suite_for_string()
{
	ut_suite* suite = ut_suite_create("string");

	ut_suite_add(suite, create, 0);
	ut_suite_add(suite, append, 0);
	ut_suite_add(suite, split, 0);

	return suite;
}
