/*
 * Copyright (c) 2024 Li Xilin <lixilin@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * This file contains the APIs that implement a memory allocation subsystem
 * based on SQLite's memsys5 memory subsystem. Refer to
 * http://www.sqlite.org/malloc.html for more info.
 *
 * This memory allocator uses the following algorithm:
 *
 *   1.  All memory allocations sizes are rounded up to a power of 2.
 *   2.  If two adjacent free blocks are the halves of a larger block,
 *       then the two blocks are coalesed into the single larger block.
 *   3.  New memory is allocated from the first available free block.
 *
 * Let n be the size of the largest allocation divided by the minimum
 * allocation size (after rounding all sizes up to a power of 2.)  Let M
 * be the maximum amount of memory ever outstanding at one time.  Let
 * N be the total amount of memory available for allocation.  Robson
 * proved that this memory allocator will never breakdown due to
 * fragmentation as long as the following constraint holds:
 *
 *      N >=  M*(1 + log2(n)/2) - n + 1
 */

#ifndef AX_MPOOL_H
#define AX_MPOOL_H

#include "lock.h"
#include "dump.h"
#include <stdint.h>

/* Maximum size of any allocation is ((1 << @ref AX_MPOOL_LOGMAX) *
 * ax_mpool_t.atom_size). Since ax_mpool_t.atom_size is always at least 8 and
 * 32-bit integers are used, it is not actually possible to reach this
 * limit.  */
#define AX_MPOOL_LOGMAX 30

/* Maximum allocation size of this memory pool library. All allocations
 * must be a power of two and must be expressed by a 32-bit signed
 * integer. Hence the largest allocation is 0x40000000 or 1073741824. */
#define AX_MPOOL_MAX_ALLOC_SIZE 0x40000000

#define AX_MPOOL_ENABLE_STATISTICS

typedef struct ax_mpool_st ax_mpool;

struct ax_mpool_st {
	size_t atom_size; /* Smallest possible allocation in bytes */
	size_t nblocks; /* Number of atom_size sized blocks in pool_buf */
	uint8_t *pool_buf; /* Memory available to be allocated */
	ax_lock lock;

#ifdef AX_MPOOL_ENABLE_STATISTICS
	/* Performance statistics */
	uint64_t n_alloc; /* Total number of calls to malloc */
	uint64_t total_alloc; /* Total of all malloc calls - includes internal fragmentation */
	uint64_t total_excess; /* Total internal fragmentation */
	uint32_t cur_out; /* Current checkout, including internal fragmentation */
	uint32_t cur_cnt; /* Current number of distinct checkouts */
	uint32_t max_out; /* Maximum instantaneous cur_out */
	uint32_t max_count; /* Maximum instantaneous cur_cnt */
	uint32_t max_req; /* Largest allocation (exclusive of internal frag) */
#endif

	/* List of free blocks.
	 * free_list[0] is a list of free blocks of size ax_mpool_t.atom_size.
	 * free_list[1] holds blocks of size atom_size * 2 and so forth. */
	int32_t free_list[AX_MPOOL_LOGMAX + 1];

	/* Space for tracking which blocks are checked out and the
	 * size of each block.  One byte per block.  */
	uint8_t *ctrl_table; 
};

/* min_alloc: Minimum size of an allocation. Any call to
 * ax_mpool_malloc where nbytes is less than min_alloc will
 * be rounded up to min_alloc. min_alloc must be a power of two.  */
int ax_mpool_init(ax_mpool *mp, void *buf, size_t buf_size, size_t min_alloc);

void *ax_mpool_malloc(ax_mpool *mp, size_t size);

void *ax_mpool_realloc(ax_mpool *mp, void *ptr, size_t size);

void ax_mpool_free(ax_mpool *mp, void *ptr);

size_t ax_mpool_roundup(ax_mpool *mp, size_t size);

ax_dump *ax_mpool_stats(const ax_mpool *mp);

#endif
