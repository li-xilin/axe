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

#include "axut/case.h"
#include <ax/mem.h>
#include <stdlib.h>
#include <string.h>

static void case_free(void *p);
static bool case_less(const void *p1, const void *p2, size_t size);
static bool case_equal(const void *p1, const void *p2, size_t size);
static ax_fail case_copy(void* dst, const void* src, size_t size);

const ax_trait axut_case_tr =
{
	.copy = case_copy,
	.init = ax_trait_mem_init,
	.less = case_less,
	.equal = case_equal,
	.free = case_free,
	.size = sizeof(axut_case),
	.link = false
};

static void case_free(void *p)
{
	axut_case *tc = p;
	free(tc->name);
	free(tc->file);
	free(tc->log);
}

static bool case_less(const void *p1, const void *p2, size_t size)
{

	const axut_case *tc1 = p1;
	const axut_case *tc2 = p2;
	return tc1->priority < tc2->priority;
}

static bool case_equal(const void *p1, const void *p2, size_t size)
{

	const axut_case *tc1 = p1;
	const axut_case *tc2 = p2;
	return tc1->priority == tc2->priority;
}

static ax_fail case_copy(void* dst, const void* src, size_t size)
{
	const axut_case *src_tc = src;
	axut_case *dst_tc = dst;
	memcpy(dst_tc, src_tc, sizeof *dst_tc);
	dst_tc->name = dst_tc->log = dst_tc->file = NULL;
	dst_tc->name = ax_strdup(src_tc->name);
	if (!src_tc->name)
		goto out;

	if (src_tc->log) {
		dst_tc->log =  ax_strdup(src_tc->log);
		if (!dst_tc->log)
			goto out;

	}
	if (src_tc->file) {
		dst_tc->file =  ax_strdup(src_tc->file);
		if (!dst_tc->file)
			goto out;
	}

	return false;
out:
	free(dst_tc->name);
	free(dst_tc->log);
	free(dst_tc->file);
	return true;
}
