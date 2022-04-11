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

#include "ax/algo.h"
#include "ax/type/one.h"
#include "ax/vector.h"
#include "ax/pred.h"
#include "ax/oper.h"
#include "ax/mem.h"
#include "ax/trait.h"
#include "ax/class.h"
#include "check.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT_ITER_TYPE(_it, _type) \
	ax_assert(((_it)->tr->type & (_type)) == (_type), \
		"expect iterator compatible with %s, but actually %s", \
		ax_iter_type_str(_type), \
		ax_iter_type_str((_it)->tr->type) \
	)

#define ASSERT_OBJ_TYPE(_obj, _type) \
	ax_assert(ax_one_is(_obj, _type), \
		"expect container compatible with %s, but actually %s", \
		_type, \
		ax_one_name((const ax_one *)_obj) \
	)

void ax_transform(const ax_citer *first1, const ax_citer *last1, const ax_iter *first2, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first1, last1);
	ax_citer cur1 = *first1;
	ax_iter cur2 = *first2;
	for (; !ax_citer_equal(&cur1, last1); ax_citer_next(&cur1)) {
		ax_pred_do(upred, ax_iter_get(&cur2), ax_citer_get(&cur1), NULL);
		ax_iter_next(&cur2);
	}
}

bool ax_all_of(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	bool out;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		if (!out)
			return false;
	}
	return true;
}

bool ax_any_of(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	bool out;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		if (out)
			return true;
	}
	return false;
}

bool ax_none_of(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	bool out;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		if (out)
			return false;
	}
	return true;
}

size_t ax_count_if(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	size_t count = 0;
	bool out = false;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		count += out ? 1 : 0;
	}
	return count;
}

ax_iter ax_search_of(const ax_iter *first1, const ax_iter *last1, const ax_citer *first2, const ax_citer *last2)
{
	CHECK_ITER_COMPARABLE(first1, last1);

	const ax_trait *etr = first1->etr;
	for (ax_iter it1 = *first1; !ax_iter_equal(&it1, last1); ax_iter_next(&it1)) {
		for (ax_citer it2 = *first2; !ax_citer_equal(&it2, last2); ax_citer_next(&it2))
		{
			void *e1 = ax_iter_get(&it1);
			const void *e2 = ax_citer_get(&it2);
			if (ax_trait_equal(etr, e1, e2)) {
				return it1; 
			}
		}
	}
	return *last1;
}

void ax_generate(const ax_iter *first, const ax_iter *last, const void *ptr)
{
	CHECK_ITER_COMPARABLE(first, last);

	const ax_trait *etr = first->etr;

	for (ax_iter it = *first; !ax_iter_equal(&it, last); ax_iter_next(&it)) {
		void *p = ax_iter_get(&it);
		ax_trait_copy(etr, p, ptr);
	}
}

void ax_find_if(ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	bool retval;
	while (!ax_citer_equal(first, last)) {
		if (ax_pred_do(upred, &retval, ax_citer_get(first), NULL), retval)
			return;
		ax_citer_next(first);
	}
}

void ax_find_if_not(ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	bool retval;
	while (!ax_citer_equal(first, last)) {
		if (ax_pred_do(upred, &retval, ax_citer_get(first), NULL), !retval)
			return;
		ax_citer_next(first);
	}
}

bool ax_sorted(const ax_citer *first, const ax_citer *last, const ax_pred *bpred)
{
	CHECK_PARAM_NULL(bpred);
	CHECK_ITER_COMPARABLE(first, last);

	if (ax_citer_equal(first, last))
		return true;

	bool retval;
	ax_citer it1 = *first, it2 = it1;
	ax_citer_next(&it2);

	while (!ax_citer_equal(&it2, last)) {
		if (ax_pred_do(bpred, &retval, ax_citer_get(&it1), ax_citer_get(&it2)), !retval)
			return false;
		it1 = it2;
		ax_citer_next(&it2);
	}
	return true;
}

void ax_partition(ax_iter *first, const ax_iter *last, const ax_pred *pred)
{
	CHECK_PARAM_NULL(pred);
	CHECK_ITER_COMPARABLE(first, last);
	ASSERT_ITER_TYPE(first, AX_IT_FORW);

	ax_find_if_not(ax_iter_c(first), ax_iter_cc(last), pred);

	if (ax_iter_equal(first, last))
		return;

	ax_iter it = *first;
	for (ax_iter_next(&it); !ax_iter_equal(&it, last); ax_iter_next(&it)) {
		bool retval;
		ax_pred_do(pred, &retval, ax_iter_get(&it), NULL);
		if (retval) {
			ax_iter_swap(&it, first);
			ax_iter_next(first);
		}
	}
}

static void less_then(bool *out, void *in1, void *in2, const ax_trait *tr)
{
	*out = ax_trait_less(tr, in1, in2);
}

static void quick_sort(const ax_iter *first, const ax_iter *last)
{
	if (ax_iter_equal(first, last))
		return;

	ax_iter second = *first;
	ax_iter_next(&second);

	if (ax_iter_equal(&second, last))
		return;

	ax_iter right_first = second;
	const ax_pred pred = ax_pred_binary_make((ax_binary_f) less_then,
			NULL, ax_iter_get(first), (void *)first->etr);
	ax_partition(&right_first, last, &pred);

	ax_iter left_last = right_first;
	ax_iter_prev(&left_last);
	ax_memswp(left_last.tr->get(ax_iter_cc(&left_last)),
			first->tr->get(ax_iter_cc(first)), first->etr->size);

	quick_sort(first, &left_last);
	quick_sort(&right_first, last);
}

ax_fail ax_quick_sort(const ax_iter *first, const ax_iter *last)
{
	CHECK_ITER_COMPARABLE(first, last);
	ASSERT_ITER_TYPE(first, AX_IT_BID);

	quick_sort(first, last);
	return false;
}

void ax_merge(const ax_citer *first1, const ax_citer *last1, const ax_citer *first2, const ax_citer *last2, ax_iter *dest)
{
	CHECK_ITER_COMPARABLE(first1, last1);
	CHECK_ITER_COMPARABLE(first2, last2);
	ASSERT_ITER_TYPE(first1, AX_IT_FORW);
	ASSERT_ITER_TYPE(first2, AX_IT_FORW);

	const ax_trait *etr = first1->etr;
	ax_citer cur1 = *first1, cur2 = *first2;

	ax_citer src;
	ax_citer src_last;
	while (!ax_citer_equal(&cur1, last1) && !ax_citer_equal(&cur2, last2)) {
		if (ax_trait_less(etr, ax_citer_get(&cur1), ax_citer_get(&cur2))) {
			src = cur1;
			ax_citer_next(&cur1);
		} else {
			src = cur2;
			ax_citer_next(&cur2);
		}
		ax_iter_set(dest, ax_citer_get(&src));

		ax_iter_next(dest);

	}

	if (!ax_citer_equal(&cur1, last1)) {
		src = cur1;
		src_last = *last1;
	} else {
		src = cur2;
		src_last = *last2;
	}
	while (!ax_citer_equal(&src, &src_last)) {
		ax_iter_set(dest, ax_citer_get(&src));
		ax_citer_next(&src);
		ax_iter_next(dest);
	}
}

struct merge_sort_context_st {
	void **imap;
	void *main;
	void *aux;
};

void merge_sort(size_t left, size_t right, struct merge_sort_context_st *ext)
{
	if (right - left <= 1)
		return;
	size_t mid = left + (right - left) / 2;
	size_t lmid = left + (mid - left) / 2;
	size_t rmid = mid + (right - mid) / 2;

	merge_sort(left, lmid, ext);
	merge_sort(lmid, mid, ext);
	merge_sort(mid, rmid, ext);
	merge_sort(rmid, right, ext);

	ax_iter end = ax_box_end(ax_r(seq, ext->main).box);
	ax_iter main_first = { .owner = end.owner, .tr = end.tr, .etr = end.etr };
	ax_iter main_mid  = { .owner = end.owner, .tr = end.tr, .etr = end.etr };
	ax_iter main_last = { .owner = end.owner, .tr = end.tr, .etr = end.etr };
	ax_iter aux_first;
	ax_iter aux_mid;
	ax_iter aux_last;

	main_first.point = ext->imap[left];
	main_mid.point = ext->imap[lmid];
	main_last.point = ext->imap[mid];

	aux_first = ax_seq_at(ext->aux, left);
	aux_mid = aux_first;
	ax_merge(ax_iter_c(&main_first), ax_iter_c(&main_mid),
			ax_iter_c(&main_mid), ax_iter_c(&main_last),
			&aux_mid);

	main_first.point = ext->imap[mid];
	main_mid.point = ext->imap[rmid];
	main_last.point = ext->imap[right];
	aux_last = aux_mid;
	ax_merge(ax_iter_c(&main_first), ax_iter_c(&main_mid),
			ax_iter_c(&main_mid), ax_iter_c(&main_last),
			&aux_last);

	main_first.point = ext->imap[left];
	main_last = main_first;
	ax_merge(ax_iter_c(&aux_first), ax_iter_c(&aux_mid),
			ax_iter_c(&aux_mid), ax_iter_c(&aux_last),
			&main_last);
}

ax_fail ax_merge_sort(const ax_iter *first, const ax_iter *last)
{
	CHECK_PARAM_NULL(first);
	CHECK_PARAM_NULL(last);
	CHECK_ITER_COMPARABLE(first, last);
	ASSERT_ITER_TYPE(first, AX_IT_FORW);
	ASSERT_OBJ_TYPE(first->owner, ax_class_name(3, seq));

	size_t size = 0;
	ax_iter cur = *first;
	while (!ax_iter_equal(&cur, last)) {
		size++;
		ax_iter_next(&cur);
	}
	
	void **imap = malloc((size + 1) * sizeof(void *));
	if (!imap)
		return true;
	
	size_t pos = 0;
	cur = *first;
	while (!ax_iter_equal(&cur, last)) {
		imap[pos++] = cur.point;
		ax_iter_next(&cur);
	}
	imap[pos] = cur.point;

	ax_vector_r aux = ax_new(vector, first->etr);
	if (!aux.one) {
		free(imap);
		return true;
	}
	ax_seq_trunc(aux.seq, size);

	struct merge_sort_context_st ext = {
		.imap = imap,
		.main = first->owner,
		.aux = aux.one,
	};

	merge_sort(0, size, &ext);

	free(imap);
	ax_one_free(aux.one);
	
	return false;
}

bool ax_equal_to_arr(const ax_iter *first, const ax_iter *last, void *arr, size_t size)
{
	CHECK_PARAM_VALIDITY(arr, !!arr == !!size);

	const ax_trait *tr = first->etr;
	ax_assert(size % tr->size == 0, "unexpected size");

	size_t pos = 0;
	ax_iter cur = *first;
	while (!ax_iter_equal(&cur, last) && pos < size) {
		if (!ax_trait_equal(tr, ax_iter_get(&cur), (ax_byte*)arr + pos))
			return false;
		ax_iter_next(&cur);
		pos += tr->size;
	}
	return ax_iter_equal(&cur, last) == (pos == size);
}

void ax_binary_search(ax_citer *first, const ax_citer *last, const void* p)
{
	CHECK_ITER_COMPARABLE(first, last);
	ASSERT_OBJ_TYPE(first->owner, ax_class_name(3, seq));
	ASSERT_ITER_TYPE(first, AX_IT_RAND);

	void *orignal_last_citer_point = last->point;
	const ax_trait *etr = first->etr;

	ax_citer left = *first, right = *last, middle;
	while (!ax_citer_equal(&left, &right)) {
		long length = ax_citer_dist(&left, &right);
		long m = length / 2;
		middle = left;
		ax_citer_move(&middle, m);
		if (ax_trait_equal(etr, ax_citer_get(&middle), p)) {
			first->point = middle.point;
			return;
		}

		if (ax_trait_less(etr, ax_citer_get(&middle), p)) {
			left.point = middle.point;
			ax_citer_next(&left);
		} else
			right.point = middle.point;
	}

	right.point = orignal_last_citer_point;
	first->point = right.point;
}

void ax_binary_search_if_not(ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_ITER_COMPARABLE(first, last);
	ASSERT_OBJ_TYPE(first->owner, ax_class_name(3, seq));
	ASSERT_OBJ_TYPE(first->owner, ax_class_name(3, seq));
	ASSERT_ITER_TYPE(first, AX_IT_RAND);

	ax_citer left = *first, right = *last, middle = { .owner = first->owner, .tr = first->tr, .etr = first->etr };
	while (!ax_citer_equal(&left, &right)) {
		bool ret;
		long length = ax_citer_dist(&left, &right);
		long m = length / 2;
		middle.point = left.point;
		ax_citer_move(&middle, m);
		if (ax_pred_do(upred, &ret, ax_citer_get(&middle), NULL), !ret)
			right.point = middle.point;
		else {
			left.point = middle.point;
			ax_citer_next(&left);
		}
	}

	first->point = left.point;
}

ax_fail ax_insertion_sort(const ax_iter *first, const ax_iter *last)
{
	CHECK_ITER_COMPARABLE(first, last);
	ASSERT_OBJ_TYPE(first->owner, ax_class_name(3, seq));
	ASSERT_ITER_TYPE(first, AX_IT_FORW);

	ax_pred pred;
	const ax_trait *etr = first->etr;
	void *tmp = malloc(etr->size);
	if (!tmp) {
		return true;
	}

	void (*search_if_not)(ax_citer *first, const ax_citer *last, const ax_pred *upred)
		= ax_iter_is(first, AX_IT_RAND)
		? ax_binary_search_if_not
		: ax_find_if_not;


	ax_iter cur = *first, sorted_first = *first, swap_prev = { first->owner, first->tr }, find;
	ax_iter_next(&cur);

	while (!ax_iter_equal(&cur, last)) {

		pred = ax_pred_binary_make((ax_binary_f) less_then, NULL,
				ax_iter_get(&cur), (void*)etr);
		find = sorted_first;
		search_if_not(ax_iter_c(&find), ax_iter_c(&cur), &pred);

		memcpy(tmp, cur.tr->get(ax_iter_cc(&cur)), etr->size);
		ax_iter swapit = cur;
		while(!ax_iter_equal(&find, &swapit)) {
			swap_prev.point = swapit.point;
			ax_iter_prev(&swap_prev);
			ax_iter_swap(&swapit, &swap_prev);
			swapit.point = swap_prev.point;
		}
		memcpy(find.tr->get(ax_iter_cc(&find)), tmp, etr->size);

		ax_iter_next(&cur);
	}
	free(tmp);
	return false;
}

void ax_find_first_unsorted(ax_citer *first, const ax_citer *last, const ax_pred *bpred)
{
	CHECK_PARAM_NULL(bpred);
	CHECK_ITER_COMPARABLE(first, last);
	ASSERT_OBJ_TYPE(first->owner, ax_class_name(3, seq));
	ASSERT_ITER_TYPE(first, AX_IT_FORW);

	if (ax_citer_equal(first, last)) {
		first->point = last->point;
		return;
	}

	bool retval;
	ax_citer it1 = *first, it2 = *first;
	ax_citer_next(&it2);
	while (!ax_citer_equal(&it2, last)) {
		if (ax_pred_do(bpred, &retval, ax_citer_get(&it1), ax_citer_get(&it2)), !retval) {
			first->point = it2.point;
			return;
		}
		it1.point = it2.point;
		ax_citer_next(&it2);
	}
	first->point = last->point;
}

