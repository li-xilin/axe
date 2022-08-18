/*
 * Copyright (c) 2022 Armon Dadgar
 * Copyright (c) 2022 Li hsilin <lihsilyn@gmail.com>
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

#if defined(NAME) || (!defined(AX_TPL_HEAP_H) && !defined(NAME))

#include "mem.h"
#include "debug.h"
#include "trick.h"

#define __AX_HEAP_PAGE_SIZE  4096

#define __AX_HEAP_LEFT_CHILD(i)   (((i)<<1)+1)
#define __AX_HEAP_RIGHT_CHILD(i)  (((i)<<1)+2)
#define __AX_HEAP_PARENT_ENTRY(i) (((i)-1)>>1)

#ifndef NAME
# define NAME ax_
# define HEAP_H
# define __AX_HEAP_DYNAMIC_TYPE
# define __AX_HEAP_GET_ENTRY(h, index) (h->table + (index) * h->entry_size)
# define __AX_HEAP_ENTRIES_PER_PAGE(h) (__AX_HEAP_PAGE_SIZE / h->entry_size)
# define __AX_HEAP_SWAP_ENTRY(h, p1, p2) ax_memswp(p1, p2, h->entry_size);
# define __AX_HEAP_ENTRY_ASSIGN(h, a, b) memcpy(a, b, h->entry_size)
# define __AX_HEAP_OUTER_TYPE void
# undef TYPE
# define TYPE uint8_t
#else
# define __AX_HEAP_GET_ENTRY(h, index) (h->table + index)
# define __AX_HEAP_ENTRIES_PER_PAGE(h) (__AX_HEAP_PAGE_SIZE / sizeof(TYPE))
# define __AX_HEAP_SWAP_ENTRY(h, p1, p2) ax_swap(p1, p2, TYPE);
# define __AX_HEAP_ENTRY_ASSIGN(h, a, b) *a = *b
# ifndef TYPE
#  define TYPE int
# endif
# define __AX_HEAP_OUTER_TYPE TYPE
#endif

#define AX_HEAP(tail) AX_CATENATE(NAME, heap_, tail)
#define heap_st AX_HEAP(st)
#define heap_init AX_HEAP(init)
#define heap_size AX_HEAP(size)
#define heap_insert AX_HEAP(insert)
#define heap_top AX_HEAP(top)
#define heap_pop AX_HEAP(pop)
#define heap_destroy AX_HEAP(destroy)

struct heap_st {
    bool (*compare)(const __AX_HEAP_OUTER_TYPE *, const __AX_HEAP_OUTER_TYPE *, void *ctx);
    size_t entry_cnt;
    size_t min_page_cnt;
    size_t page_cnt;
#ifdef __AX_HEAP_DYNAMIC_TYPE
	size_t entry_size;
#endif
    TYPE *table;
	void *ctx;
};

extern void *malloc(size_t);
extern void *realloc(void *, size_t);
extern void free(void *);

static ax_fail heap_init(struct heap_st* h,
#ifdef __AX_HEAP_DYNAMIC_TYPE
		size_t entry_size,
#endif
		size_t min_pages, bool (*comp_func)(const __AX_HEAP_OUTER_TYPE *, const __AX_HEAP_OUTER_TYPE *, void *ctx), void *ctx)
{
	ax_assert_not_null(h);
	ax_assert_not_null(comp_func);
	ax_assert_not_null(min_pages > 0);
    h->compare = comp_func;
    h->entry_cnt = 0;
    h->page_cnt = min_pages;
#ifdef __AX_HEAP_DYNAMIC_TYPE
	h->entry_size = entry_size;
#endif
    h->min_page_cnt = h->page_cnt;
	h->ctx = ctx;
	h->table = (TYPE *)malloc(h->page_cnt * __AX_HEAP_PAGE_SIZE);
	if (!h->table)
		return true;
	return false;
}

static void heap_destroy(struct heap_st* h)
{
    if (!h)
		return;
    free(h->table);

    h->entry_cnt = 0;
    h->page_cnt = 0;
    h->table = NULL;
}

inline static size_t heap_size(const struct heap_st* h)
{
    return h->entry_cnt;
}

static const __AX_HEAP_OUTER_TYPE *heap_top(const struct heap_st* h)
{
    if (h->entry_cnt == 0)
        return 0;
    return (__AX_HEAP_OUTER_TYPE *)__AX_HEAP_GET_ENTRY(h, 0);
}

static ax_fail heap_insert(struct heap_st* h, const __AX_HEAP_OUTER_TYPE *key)
{
    ax_assert_not_null(h->table);

    int max_entries = h->page_cnt * __AX_HEAP_ENTRIES_PER_PAGE(h);
    if (h->entry_cnt + 1 > max_entries) {
        int new_size = h->page_cnt * 2;
        TYPE *new_table = (TYPE *)realloc(h->table, new_size * __AX_HEAP_PAGE_SIZE);
		if (!new_table)
			return true;
        h->table = new_table;
        h->page_cnt = new_size;
    }
    
    int current_index = h->entry_cnt;
    TYPE *current = __AX_HEAP_GET_ENTRY(h, current_index);

    int parent_index;
    TYPE *parent;

    while (current_index > 0) {
        parent_index = __AX_HEAP_PARENT_ENTRY(current_index);
        parent = __AX_HEAP_GET_ENTRY(h, parent_index);
        if (h->compare(key, parent, h->ctx)) {
			__AX_HEAP_ENTRY_ASSIGN(h, current, parent);
            current_index = parent_index;
			current = parent;

        } else
            break;
    }
	__AX_HEAP_ENTRY_ASSIGN(h, current, key);
    h->entry_cnt++;
	return false;
}

static void heap_pop(struct heap_st* h)
{
    ax_assert_not_null(h->entry_cnt);
    int current_index = 0;
    TYPE *current = __AX_HEAP_GET_ENTRY(h, current_index);
    h->entry_cnt--;
    int entries = h->entry_cnt;
    if (h->entry_cnt > 0) {
        TYPE *last = __AX_HEAP_GET_ENTRY(h, entries);
		__AX_HEAP_ENTRY_ASSIGN(h, current, last);

        TYPE *left_child, *right_child;
        bool (*cmp_func)(const __AX_HEAP_OUTER_TYPE *, const __AX_HEAP_OUTER_TYPE *, void *ctx) = h->compare;
        int left_child_index;
        while (left_child_index = __AX_HEAP_LEFT_CHILD(current_index), left_child_index < entries) {
            left_child = __AX_HEAP_GET_ENTRY(h, left_child_index);

            if (left_child_index+ 1 < entries) {
                right_child = __AX_HEAP_GET_ENTRY(h, left_child_index+1);
                if (!cmp_func(right_child, left_child, h->ctx)) {
                    if (cmp_func(left_child, current, h->ctx)) {
						__AX_HEAP_SWAP_ENTRY(h, current, left_child);
                        current_index = left_child_index;
                        current = left_child;
                    } else
                        break;
                } else {
                    if (cmp_func(right_child, current, h->ctx)) {
						__AX_HEAP_SWAP_ENTRY(h, current, right_child);
                        current_index = left_child_index+1;
                        current = right_child;
                    } else
                        break;
                }
            } else if (cmp_func(left_child, current, h->ctx)) {
				__AX_HEAP_SWAP_ENTRY(h, current, left_child);
                current_index = left_child_index;
                current = left_child;

            }  else
                break;
        }
    } 

    int used_pages = entries / __AX_HEAP_ENTRIES_PER_PAGE(h) + ((entries % __AX_HEAP_ENTRIES_PER_PAGE(h) > 0) ? 1 : 0);
    if (h->page_cnt / 2 > used_pages + 1 && h->page_cnt / 2 >= h->min_page_cnt) {
        int new_size = h->page_cnt / 2;
        TYPE *new_table = (TYPE *)realloc(h->table, new_size * __AX_HEAP_PAGE_SIZE);
		if (new_table)
			h->table = new_table;

        h->table = new_table;
        h->page_cnt = new_size;
    }
}

#undef AX_HEAP
#undef heap_st
#undef heap_init
#undef heap_size
#undef heap_insert
#undef heap_top
#undef heap_pop
#undef heap_destroy

#undef __AX_HEAP_GET_ENTRY
#undef __AX_HEAP_ENTRIES_PER_PAGE
#undef __AX_HEAP_SWAP_ENTRY
#undef __AX_HEAP_ENTRY_ASSIGN
#undef __AX_HEAP_OUTER_TYPE
#undef __AX_HEAP_PAGE_SIZE
#undef __AX_HEAP_DYNAMIC_TYPE

#undef NAME
#undef TYPE

#endif
