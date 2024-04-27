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
#include <time.h>

#define RANDOM_NALLOC 512
#define PERF_NALLOC 4096

static void check_data(ut_runner *r, char **list, size_t *size, size_t n)
{
	for (int i = 0; i < n; i++) {
		char expect[256];
		memset(expect, size[i], size[i]);
		ut_assert_mem_equal(r, expect, size[i], list[i], size[i]);
	}
}

static void random_test(ut_runner *r)
{
	char *buf = malloc(RANDOM_NALLOC * 256);
	if (!buf)
		ut_term(r, "malloc");
	ax_mpool mp;
	ax_mpool_init(&mp, buf, RANDOM_NALLOC * 256, 4);

	size_t *size_list = malloc(RANDOM_NALLOC * sizeof(size_t));
	char **list = malloc(RANDOM_NALLOC * sizeof(void *));
	size_t list_len = 0;

	for (int i = 0; i < 1000; i++) {
		if (list_len >= RANDOM_NALLOC) {
			/* To randomly free a half of allocated memory block */
			for (int j = 0; j < RANDOM_NALLOC / 2; j++) {
				int idx = rand() % list_len;
				ax_mpool_free(&mp, list[idx]);
				list[idx] = list[list_len - 1];
				size_list[idx] = size_list[list_len - 1];
				list_len--;
				check_data(r, list, size_list, list_len);
			}
		}
		else {
			size_list[list_len] = rand() % 0xFF;
			list[list_len] = ax_mpool_malloc(&mp, size_list[list_len]);
			memset(list[list_len], size_list[list_len], size_list[list_len]);
			list_len++;
			check_data(r, list, size_list, list_len);
		}
	}
	free(buf);
}

static void perf_test(ut_runner *r)
{
	char *buf = malloc(PERF_NALLOC * 256);
	if (!buf)
		ut_term(r, "malloc");
	ax_mpool mp;
	ax_mpool_init(&mp, buf, PERF_NALLOC * 256, 4);

	size_t *size_list = malloc(PERF_NALLOC * sizeof(size_t));
	char **list = malloc(PERF_NALLOC * sizeof(void *));
	size_t list_len = 0;


	const int nloops = 100000;

	clock_t tim = clock();
	for (int i = 0; i < nloops; i++) {
		if (list_len >= PERF_NALLOC) {
			for (int j = 0; j < PERF_NALLOC / 2; j++) {
				int idx = rand() % list_len;
				ax_mpool_free(&mp, list[idx]);
				list[idx] = list[list_len - 1];
				size_list[idx] = size_list[list_len - 1];
				list_len--;
			}
		}
		else {
			size_list[list_len] = rand() % 128;
			list[list_len] = ax_mpool_malloc(&mp, size_list[list_len]);
			list_len++;
		}
	}
	ut_printf(r, "mpool time: %dms", (clock() - tim));
	ax_dump *dmp = ax_mpool_stats(&mp);
	ax_dump_fput(dmp, ax_dump_default_format(), stderr);
	free(buf);

	tim = clock();
	list_len = 0;
	for (int i = 0; i < nloops; i++) {
		if (list_len >= PERF_NALLOC) {
			for (int j = 0; j < PERF_NALLOC / 2; j++) {
				int idx = rand() % list_len;
				free(list[idx]);
				list[idx] = list[list_len - 1];
				size_list[idx] = size_list[list_len - 1];
				list_len--;
			}
		}
		else {
			size_list[list_len] = rand() % 128;
			list[list_len] = malloc(size_list[list_len]);
			list_len++;
		}
	}
	ut_printf(r, "malloc time: %dms", (clock() - tim));

	for (int i = 0; i < list_len; i++)
		free(list[i]);
}

ut_suite *suite_for_mpool()
{
	ut_suite* suite = ut_suite_create("mpool");
	ut_suite_add(suite, random_test, 0);
	ut_suite_add(suite, perf_test, 0);

	return suite;
}
