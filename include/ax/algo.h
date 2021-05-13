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

#ifndef AX_ALGO_H
#define AX_ALGO_H
#include "def.h"
#include "iter.h"
#include "pred.h"

void ax_transform(
		const ax_citer *first1,
		const ax_citer *last1,
		const ax_iter *first2,
		const ax_pred *upred);

bool ax_all_of(
		const ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

bool ax_any_of(
		const ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

bool ax_none_of(
		const ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

size_t ax_count_if(
		const ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

ax_iter ax_search_of(
		const ax_iter *first1,
		const ax_iter *last1,
		const ax_citer *first2,
		const ax_citer *last2);

void ax_generate(
		const ax_iter *first,
		const ax_iter *last,
		const void *ptr);

void ax_find_if(
		ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

void ax_find_if_not(
		ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);


bool ax_sorted(
		const ax_citer *first,
		const ax_citer *last,
		const ax_pred *bpred);

void ax_partition(
		ax_iter *first,
		const ax_iter *last,
		const ax_pred *upred);

ax_fail ax_quick_sort(
		const ax_iter *first,
		const ax_iter *last);

bool ax_equal_to_arr(
		const ax_iter *first, 
		const ax_iter *last, 
		void *arr, 
		size_t arrlen);

void ax_merge(
		const ax_citer *first1,
		const ax_citer *last1,
		const ax_citer *first2,
		const ax_citer *last2,
		ax_iter *dest);

ax_fail ax_merge_sort(
		const ax_iter *first,
		const ax_iter *last);

void ax_binary_search(
		ax_citer *first, 
		const ax_citer *last,
		const void *p);

void ax_binary_search_if_not(
		ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

ax_fail ax_insertion_sort(
		const ax_iter *first,
		const ax_iter *last);

void ax_find_first_unsorted(
		ax_citer *first,
		const ax_citer *last,
		const ax_pred *bpred);

#endif
