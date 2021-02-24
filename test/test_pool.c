#include "axe/pool.h"

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
	printf("test memory pool spent %lfs\n", (double)(clock() - begin) / CLOCKS_PER_SEC);
	free(table);
	
}

axut_suite *suite_for_pool(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "pool");
	srand(42);

	axut_suite_add(suite, pool, 0);

	return suite;
}
