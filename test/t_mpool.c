/*
 * Copyright (c) 2024 Li hsilin <lihsilyn@gmail.com>
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

#include "ax/mpool.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void common(ut_runner *r)
{
	char *buf = malloc(40960);
	if (!buf)
		ut_term(r, "malloc");
	ax_mpool mp;
	ax_mpool_init(&mp, buf, 40960, 4);

	char **list = malloc(10240 * sizeof(void *));
	size_t list_len = 0;

	for (int i = 0; i < 4096; i++) {
		if (list_len >= 10240) {
			continue;
		}
		if (rand() % 2 && list_len != 0 && list_len < 10240) {
			int idx = rand() % list_len;
			ax_mpool_free(&mp, list[idx]);
			list[idx] = list[list_len - 1];
			list_len--;
		}
		else {
			list[list_len++] = ax_mpool_malloc(&mp, rand() % 1024);
			
		}
	}
}
ut_suite *suite_for_mpool()
{
	ut_suite* suite = ut_suite_create("mpool");

	ut_suite_add(suite, common, 0);

	return suite;
}
