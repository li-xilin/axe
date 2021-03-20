#ifndef AXE_ALGO_H_
#define AXE_ALGO_H_
#include "def.h"
#include "iter.h"
#include "pred.h"

void ax_transform(
		const ax_citer *first1,
		const ax_citer *last1,
		const ax_iter *first2,
		const ax_pred *upred);

ax_bool ax_all_of(
		const ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

ax_bool ax_any_of(
		const ax_citer *first,
		const ax_citer *last,
		const ax_pred *upred);

ax_bool ax_none_of(
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


ax_bool ax_sorted(
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

ax_bool ax_equal_to_arr(
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
