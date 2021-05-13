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

#include "ax/pool.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#define ALLOC_COUNT 0xFFFFF
static void pool(axut_runner *r)
{
	long begin = clock();
	ax_pool *pool = ax_pool_create();
	unsigned char **table = malloc(ALLOC_COUNT * sizeof(void*));
	for (int i = 0; i < ALLOC_COUNT; i++) {
		table[i] = ax_pool_alloc(pool, i%0x100);
		memset(table[i], i%0xFF, i%0x100);
	}
	for (int i = 0; i < ALLOC_COUNT; i++) {
		for (int j = 0; j < i%0x100; j++)
			axut_assert(r, table[i][j] == i%0xFF);
	}
	for (int i = 0; i < ALLOC_COUNT; i++) {
		ax_pool_free(table[i]);
	}
	ax_pool_destroy(pool);
	//printf("test memory pool spent %lfs\n", (double)(clock() - begin) / CLOCKS_PER_SEC);
	free(table);
	
}

axut_suite *suite_for_pool(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "pool");
	srand(42);

	axut_suite_add(suite, pool, 0);

	return suite;
}
