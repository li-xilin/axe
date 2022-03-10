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

#include "assist.h"

#include "ax/mem.h"
#include "axut.h"

#include <stdlib.h>

static void strsplit(ax_runner *r)
{
	char str[] = "a|bb|ccc|dddd||e|";
	char *next = str, *p;
	p = ax_strsplit(&next, '|');
	axut_assert_str_equal(r, "a", p);
	p = ax_strsplit(&next, '|');
	axut_assert_str_equal(r, "bb", p);
	p = ax_strsplit(&next, '|');
	axut_assert_str_equal(r, "ccc", p);
	p = ax_strsplit(&next, '|');
	axut_assert_str_equal(r, "dddd", p);
	p = ax_strsplit(&next, '|');
	axut_assert_str_equal(r, "", p);
	p = ax_strsplit(&next, '|');
	axut_assert_str_equal(r, "e", p);
	p = ax_strsplit(&next, '|');
	axut_assert_str_equal(r, "", p);
	p = ax_strsplit(&next, '|');
	axut_assert(r, p == NULL);
}

static void strrepl(ax_runner *r)
{
	char *res = NULL;

	res = ax_strrepl("a", "a", "");
	axut_assert_str_equal(r, "", res);
	free(res);

	res = ax_strrepl("a", "b", "");
	axut_assert_str_equal(r, "a", res);
	free(res);

	res = ax_strrepl("", "a", "");
	axut_assert_str_equal(r, "", res);
	free(res);

	res = ax_strrepl("a", "ab", "");
	axut_assert_str_equal(r, "a", res);
	free(res);

	res = ax_strrepl("a", "a", "b");
	axut_assert_str_equal(r, "b", res);
	free(res);

	res = ax_strrepl("a", "a", "ab");
	axut_assert_str_equal(r, "ab", res);
	free(res);

	res = ax_strrepl("ab", "b", "c");
	axut_assert_str_equal(r, "ac", res);
	free(res);

	res = ax_strrepl("aabccCeefgg", "cc", "CC");
	axut_assert_str_equal(r, "aabCCCeefgg", res);
	free(res);

}

axut_suite *suite_for_mem()
{
	axut_suite* suite = axut_suite_create("mem");
	axut_suite_add(suite, strsplit, 0);
	axut_suite_add(suite, strrepl, 0);
	return suite;
}
