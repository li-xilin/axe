#ifndef AXE_ALGO_H_
#define AXE_ALGO_H_
#include "def.h"
#include "iter.h"
#include "pred.h"

void ax_transform(ax_iter first1, ax_iter last1, ax_iter first2, const ax_pred *upred);

ax_bool ax_all_of(ax_iter first, ax_iter last, const ax_pred *upred);

ax_bool ax_any_of(ax_iter first, ax_iter last, const ax_pred *upred);

ax_bool ax_none_of(ax_iter first, ax_iter last, const ax_pred *upred);

size_t ax_count_if(ax_iter first, ax_iter last, const ax_pred *upred);

ax_iter ax_search_of(ax_iter first1, ax_iter last1, ax_iter first2, ax_iter last2);

void ax_generate(ax_iter first, ax_iter last, const void *ptr);

inline static ax_iter ax_find_if(ax_iter first, ax_iter last, const ax_pred *upred)
{
	void __ax_find_if(ax_iter *first, const ax_iter *last, const ax_pred *upred);
	__ax_find_if(&first, &last, upred);
	return first;
}

inline static ax_iter ax_find_if_not(ax_iter first, ax_iter last, const ax_pred *upred)
{
	void __ax_find_if_not(ax_iter *first, const ax_iter *last, const ax_pred *upred);
	__ax_find_if_not(&first, &last, upred);
	return first;
}

ax_bool ax_sorted(ax_iter first, ax_iter last, const ax_pred *bpred);

inline static ax_iter ax_partition(ax_iter first, ax_iter last, const ax_pred upred)
{
	void __ax_partition(ax_iter *first, const ax_iter *last, const ax_pred *upred);
	__ax_partition(&first, &last, &upred);
	return first;
}

ax_fail ax_quick_sort(ax_iter first, ax_iter last);

ax_bool ax_equal_to_arr(ax_iter first, ax_iter last, void *arr, size_t arrlen);

inline static ax_iter ax_merge(ax_iter first1, ax_iter last1, ax_iter first2, ax_iter last2, ax_iter dest)
{
	void __ax_merge(ax_iter *first1, const ax_iter *last1, ax_iter *first2, const ax_iter *last2, ax_iter *dest);
	__ax_merge(&first1, &last1, &first2, &last2, &dest);
	return dest;
}

ax_fail ax_merge_sort(ax_iter first, ax_iter last);

ax_iter ax_binary_search(ax_iter first, ax_iter last, const void *p);

ax_iter ax_binary_search_if_not(ax_iter first, ax_iter last, const ax_pred *upred);

ax_fail ax_insertion_sort(ax_iter first, ax_iter last);

ax_iter ax_find_first_unsorted(ax_iter first, ax_iter last, const ax_pred *bpred);

#endif
