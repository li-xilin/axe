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

#include "ax/base.h"
#include "ax/mem.h"

#include "axut.h"

static void strsplit(axut_runner *r)
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

axut_suite *suite_for_mem(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "mem");
	axut_suite_add(suite, strsplit, 0);
	return suite;
}
