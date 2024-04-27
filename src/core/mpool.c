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
 */

#include "check.h"
#include "ax/mpool.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

/* A minimum allocation is an instance of the following structure.
 * Larger allocations are an array of these structures where the
 * size of the array is a power of 2.
 *
 * The size of this object must be a power of two.  That fact is
 * verified in ax_mpool_init().  */

typedef struct {
	int next;
	int prev;
} link;

/* Masks used for ax_mpool.ctrl_table[] elements.  */
#define CTRL_LOGSIZE  0x1f    /* Log2 Size of this block */
#define CTRL_FREE     0x20    /* True if not checked out */

/* Assuming ax_mpool.pool_buf is divided up into an array of mpool_link
 * structures, return a pointer to the idx-th such lik. */
#define mpool_getlink(mp, idx) ((link *)(&mp->pool_buf[(idx) * mp->atom_size]))

static int mpool_logarithm(size_t value);
static size_t mpool_size(const ax_mpool *mp, void *p);
static void mpool_link(ax_mpool *mp, int i, size_t log_size);
static void mpool_unlink(ax_mpool *mp, int i, size_t log_size);
static int mpool_unlink_first(ax_mpool *mp, size_t log_size);
static void *mpool_malloc_unsafe(ax_mpool *mp, size_t nbytes);
static void mpool_free_unsafe(ax_mpool *mp, void *old_ptr);

int ax_mpool_init(ax_mpool *mp, void *buf, size_t buf_size, size_t min_alloc)
{
	CHECK_PARAM_NULL(mp);
	CHECK_PARAM_NULL(buf);
	CHECK_PARAM_VALIDITY(buf_size, buf_size != 0);
	CHECK_PARAM_VALIDITY(buf_size, min_alloc != 0);

	size_t nbytes; /* Number of bytes of memory available to this allocator */
	size_t nminlogs; /* Log base 2 of minimum allocation size in bytes */
	size_t offset; /* An offset into mp->ctrl_table[] */

	memset(mp, 0, sizeof *mp);

	/* The size of a link object must be a power of two */
	assert((sizeof (link)&(sizeof (link) - 1)) == 0);

	if (ax_lock_init(&mp->lock))
		return -1;

	nbytes = buf_size;

	nminlogs = mpool_logarithm(min_alloc);
	mp->atom_size = (1 << nminlogs);
	while ((int) sizeof (link) > mp->atom_size)
		mp->atom_size = mp->atom_size << 1;

	mp->nblocks = (nbytes / (mp->atom_size + sizeof (uint8_t)));
	mp->pool_buf = buf;
	mp->ctrl_table = (uint8_t *) & mp->pool_buf[mp->nblocks * mp->atom_size];

	for (int i = 0; i <= AX_MPOOL_LOGMAX; i++)
		mp->free_list[i] = -1;

	offset = 0;
	for (int i = AX_MPOOL_LOGMAX; i >= 0; i--) {
		int n_alloc = (1 << i);
		if ((offset + n_alloc) <= mp->nblocks) {
			mp->ctrl_table[offset] = (uint8_t) (i | CTRL_FREE);
			mpool_link(mp, offset, i);
			offset += n_alloc;
		}
		assert((offset + n_alloc) > mp->nblocks);
	}
	return 0;
}

void *ax_mpool_malloc(ax_mpool *mp, size_t size)
{
	CHECK_PARAM_NULL(mp);
	ax_lock_get(&mp->lock);
	int64_t *p = mpool_malloc_unsafe(mp, size);
	ax_lock_put(&mp->lock);
	return (void*) p;
}

void ax_mpool_free(ax_mpool *mp, void *ptr)
{
	CHECK_PARAM_NULL(mp);
	if (!ptr)
		return;
	ax_lock_get(&mp->lock);
	mpool_free_unsafe(mp, ptr);
	ax_lock_put(&mp->lock);
	ax_lock_free(&mp->lock);
}

void *ax_mpool_realloc(ax_mpool *mp, void *ptr, size_t size)
{
	CHECK_PARAM_NULL(mp);

	size_t old_size = 0;
	if (ptr) {
		old_size = mpool_size(mp, ptr);
		if (size <= old_size)
			return (void *) ptr;
	}
	ax_lock_get(&mp->lock);
	void *p = mpool_malloc_unsafe(mp, size);
	if (p) {
		memcpy(p, ptr, old_size);
		mpool_free_unsafe(mp, ptr);
	}
	ax_lock_put(&mp->lock);
	return p;
}

size_t ax_mpool_roundup(ax_mpool *mp, size_t n)
{
	CHECK_PARAM_NULL(mp);
	assert(n <= AX_MPOOL_MAX_ALLOC_SIZE);
	size_t full_size;
	for (full_size = mp->atom_size; full_size < n; full_size *= 2);
	return full_size;
}

ax_dump *ax_mpool_stats(const ax_mpool *mp)
{

#ifdef AX_MPOOL_ENABLE_STATISTICS
	ax_dump *blk = ax_dump_block("mpool_stats", 8);
	ax_dump_bind(blk, 0, ax_dump_pair(ax_dump_symbol("NUM_OF_MALLOC_CALL"), ax_dump_uint(mp->n_alloc)));
	ax_dump_bind(blk, 1, ax_dump_pair(ax_dump_symbol("TOTAL_OF_ALL_MALLOC_CALLS"), ax_dump_uint(mp->total_alloc)));
	ax_dump_bind(blk, 2, ax_dump_pair(ax_dump_symbol("NUM_OF_FRAGMENTATIONS"), ax_dump_uint(mp->total_excess)));
	ax_dump_bind(blk, 3, ax_dump_pair(ax_dump_symbol("CURRENT_CHECKOUT"), ax_dump_uint(mp->cur_out)));
	ax_dump_bind(blk, 4, ax_dump_pair(ax_dump_symbol("CUR_NUM_OF_DISTINCE_CHECKOUTS"), ax_dump_uint(mp->cur_cnt)));
	ax_dump_bind(blk, 5, ax_dump_pair(ax_dump_symbol("MAX_INSTANTANEOUS_CUR_OUT"), ax_dump_uint(mp->max_out)));
	ax_dump_bind(blk, 6, ax_dump_pair(ax_dump_symbol("MAX_INSTANTANEOUS_CUR_CNT"), ax_dump_uint(mp->max_count)));
	ax_dump_bind(blk, 7, ax_dump_pair(ax_dump_symbol("MAX_ALLOC_SIZE"), ax_dump_uint(mp->max_req)));
	return blk;
#else
	ax_dump *blk = ax_dump_block("mpool_stats", 1);
	ax_dump_bind(blk, 0, ax_dump_symbol("DISABLED"));
	return blk;
#endif
}

/* Return the ceiling of the logarithm base 2 of value.
 *
 * Examples:   mpool_logarithm(1) -> 0
 *             mpool_logarithm(2) -> 1
 *             mpool_logarithm(4) -> 2
 *             mpool_logarithm(5) -> 3
 *             mpool_logarithm(8) -> 3
 *             mpool_logarithm(9) -> 4 */
static int mpool_logarithm(size_t value)
{
	int i;
	for (i = 0; (1 << i) < value; i++);
	return i;
}

/* Return the size of an outstanding allocation, in bytes. The
 * size returned omits the 8-byte header overhead. This only
 * works for chunks that are currently checked out. */
static size_t mpool_size(const ax_mpool *mp, void *p)
{
	if (!p)
		return 0;
	int size = 0;
	int i = ((uint8_t *) p - mp->pool_buf) / mp->atom_size;
	assert(i >= 0 && i < mp->nblocks);
	size = mp->atom_size *
		(1 << (mp->ctrl_table[i] & CTRL_LOGSIZE));
	return size;
}

/* Link the chunk at mp->aPool[i] so that is on the log_size
 * free list. */
static void mpool_link(ax_mpool *mp, int i, size_t log_size)
{
	int x;
	assert(i >= 0 && i < mp->nblocks);
	assert(log_size >= 0 && log_size <= AX_MPOOL_LOGMAX);
	assert((mp->ctrl_table[i] & CTRL_LOGSIZE) == log_size);

	x = mpool_getlink(mp, i)->next = mp->free_list[log_size];
	mpool_getlink(mp, i)->prev = -1;
	if (x >= 0) {
		assert(x < mp->nblocks);
		mpool_getlink(mp, x)->prev = i;
	}
	mp->free_list[log_size] = i;
}

/* Unlink the chunk at mp->aPool[i] from list it is currently
 * on. It should be found on mp->free_list[log_size]. */
static void mpool_unlink(ax_mpool *mp, int i, size_t log_size)
{
	int next, prev;
	assert(i >= 0 && i < mp->nblocks);
	assert(log_size >= 0 && log_size <= AX_MPOOL_LOGMAX);
	assert((mp->ctrl_table[i] & CTRL_LOGSIZE) == log_size);

	next = mpool_getlink(mp, i)->next;
	prev = mpool_getlink(mp, i)->prev;
	if (prev < 0)
		mp->free_list[log_size] = next;
	else
		mpool_getlink(mp, prev)->next = next;

	if (next >= 0)
		mpool_getlink(mp, next)->prev = prev;
}

/* Find the first entry on the freelist log_size. Unlink that
 * entry and return its index. */
static int mpool_unlink_first(ax_mpool *mp, size_t log_size)
{
	int i, first;
	assert(log_size >= 0 && log_size <= AX_MPOOL_LOGMAX);
	i = first = mp->free_list[log_size];
	assert(first >= 0);
	while (i > 0) {
		if (i < first) first = i;
		i = mpool_getlink(mp, i)->next;
	}
	mpool_unlink(mp, first, log_size);
	return first;
}

/* Return a block of memory of at least nbytes in size.
 * Return NULL if unable.  Return NULL if nbytes==0.
 *
 * The caller guarantees that nbytes positive.
 *
 * The caller has obtained a lock prior to invoking this
 * routine so there is never any chance that two or more
 * threads can be in this routine at the same time.  */
static void *mpool_malloc_unsafe(ax_mpool *mp, size_t nbytes)
{
	int i; /* Index of a mp->aPool[] slot */
	int bin; /* Index into mp->free_list[] */
	size_t full_size; /* Size of allocation rounded up to power of 2 */
	size_t log_size; /* Log2 of full_size/POW2_MIN */

	/* Keep track of the maximum allocation request.  Even unfulfilled
	 * requests are counted */
#ifdef AX_MPOOL_ENABLE_STATISTICS
	if ((uint32_t) nbytes > mp->max_req) {
		mp->max_req = nbytes;
	}
#endif

	/* Abort if the requested allocation size is larger than the largest
	 * power of two that we can represent using 32-bit signed integers. */
	if (nbytes > AX_MPOOL_MAX_ALLOC_SIZE) {
		errno = EINVAL;
		return NULL;
	}

	/* Round nbytes up to the next valid power of two */
	for (full_size = mp->atom_size, log_size = 0;
			full_size < nbytes; full_size *= 2, log_size++);

	/* Make sure mp->free_list[log_size] contains at least one free
	 * block.  If not, then split a block of the next larger power of
	 * two in order to create a new free block of size log_size. */
	for (bin = log_size; mp->free_list[bin] < 0 && bin <= AX_MPOOL_LOGMAX; bin++);

	if (bin > AX_MPOOL_LOGMAX) {
		errno = ENOMEM;
		return NULL;
	}

	i = mpool_unlink_first(mp, bin);
	while (bin > log_size) {
		bin--;
		size_t new_size = 1 << bin;
		mp->ctrl_table[i + new_size] = (uint8_t) (CTRL_FREE | bin);
		mpool_link(mp, i + new_size, bin);
	}
	mp->ctrl_table[i] = (uint8_t) log_size;

#ifdef AX_MPOOL_ENABLE_STATISTICS
	/* Update allocator performance statistics. */
	mp->n_alloc++;
	mp->total_alloc += full_size;
	mp->total_excess += full_size - nbytes;
	mp->cur_cnt++;
	mp->cur_out += full_size;
	if (mp->max_count < mp->cur_cnt)
		mp->max_count = mp->cur_cnt;
	if (mp->max_out < mp->cur_out)
		mp->max_out = mp->cur_out;
#endif

	return (void*)&mp->pool_buf[i * mp->atom_size];
}

/* Free an outstanding memory allocation. */
static void mpool_free_unsafe(ax_mpool *mp, void *ptr)
{
	uint32_t size, log_size;
	int block;

	if (!ptr)
		return;

	/* Set block to the index of the block pointed to by ptr in
	 * the array of mp->atom_size byte blocks pointed to by mp->pool_buf.  */
	block = ((uint8_t *) ptr - mp->pool_buf) / mp->atom_size;

	/* Check that the pointer ptr points to a valid, non-free block. */
	assert(block >= 0 && block < mp->nblocks);
	assert(((uint8_t *) ptr - mp->pool_buf) % mp->atom_size == 0);
	assert((mp->ctrl_table[block] & CTRL_FREE) == 0);

	log_size = mp->ctrl_table[block] & CTRL_LOGSIZE;
	size = 1 << log_size;
	assert(block + size - 1 < (uint32_t) mp->nblocks);

	mp->ctrl_table[block] |= CTRL_FREE;
	mp->ctrl_table[block + size - 1] |= CTRL_FREE;

#ifdef AX_MPOOL_ENABLE_STATISTICS
	assert(mp->cur_cnt > 0);
	assert(mp->cur_out >= (size * mp->atom_size));
	mp->cur_cnt--;
	mp->cur_out -= size * mp->atom_size;
	assert(mp->cur_out > 0 || mp->cur_cnt == 0);
	assert(mp->cur_cnt > 0 || mp->cur_out == 0);
#endif

	mp->ctrl_table[block] = (uint8_t) (CTRL_FREE | log_size);
	while (log_size < AX_MPOOL_LOGMAX) {
		int buddy;
		if ((block >> log_size) & 1) {
			buddy = block - size;
		}
		else {
			buddy = block + size;
		}
		assert(buddy >= 0);
		if ((buddy + (1 << log_size)) > mp->nblocks) break;
		if (mp->ctrl_table[buddy] != (CTRL_FREE | log_size)) break;
		mpool_unlink(mp, buddy, log_size);
		log_size++;
		if (buddy < block) {
			mp->ctrl_table[buddy] = (uint8_t) (CTRL_FREE | log_size);
			mp->ctrl_table[block] = 0;
			block = buddy;
		}
		else {
			mp->ctrl_table[block] = (uint8_t) (CTRL_FREE | log_size);
			mp->ctrl_table[buddy] = 0;
		}
		size *= 2;
	}
	mpool_link(mp, block, log_size);
}
