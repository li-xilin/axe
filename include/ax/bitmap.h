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

#ifndef AX_BITMAP_H
#define AX_BITMAP_H

#ifndef AX_BITMAP_DEFINED
#define AX_BITMAP_DEFINED
typedef struct ax_bitmap_st ax_bitmap;
#endif

#include <ax/debug.h>
#include <stdint.h>
#include <string.h>

struct ax_bitmap_st
{
	size_t nbytes;
	uint8_t *data;
};

inline static void ax_bitmap_init(ax_bitmap *bm, void *data, size_t size)
{
	bm->data = data;
	bm->nbytes = size;
}

inline static int ax_bitmap_get(ax_bitmap *bm, size_t idx)
{
	ax_assert(idx < bm->nbytes * 8, "Index out of bounds");
	int byte_idx = idx / 8, bit_idx = idx % 8;
	return (bm->data[byte_idx] >> bit_idx) & 1;
}

inline static void ax_bitmap_set(ax_bitmap *bm, size_t idx, int bit)
{
	ax_assert(idx < bm->nbytes * 8, "Index out of bounds");
	int byte_idx = idx / 8, bit_idx = idx % 8;
	uint8_t old_bit = (bm->data[byte_idx] >> bit_idx) & 1;
	bm->data[byte_idx] ^= (old_bit ^ !!bit) << bit_idx;
}

inline static void ax_bitmap_clear(ax_bitmap *bm, int val)
{
	memset(bm->data, !!val * 0xFF, bm->nbytes);
}

inline static void ax_bitmap_and(ax_bitmap *bm, size_t idx, int bit)
{
	ax_assert(idx < bm->nbytes * 8, "Index out of bounds");
	int byte_idx = idx / 8, bit_idx = idx % 8;
	bm->data[byte_idx] &=  ~(!bit << bit_idx);
}

inline static void ax_bitmap_or(ax_bitmap *bm, size_t idx, int bit)
{
	ax_assert(idx < bm->nbytes * 8, "Index out of bounds");
	int byte_idx = idx / 8, bit_idx = idx % 8;
	bm->data[byte_idx] |=  !!bit << bit_idx;
}

inline static void ax_bitmap_toggle(ax_bitmap *bm, size_t idx)
{
	ax_assert(idx < bm->nbytes * 8, "Index out of bounds");
	int byte_idx = idx / 8, bit_idx = idx % 8;
	bm->data[byte_idx] ^=  (1 << bit_idx);
}

int ax_bitmap_find(ax_bitmap *bm, size_t start, int bit);

size_t ax_bitmap_count(ax_bitmap *bm);

#endif
