#include "check.h"
#include <axe/algo.h>
#include <axe/vector.h>
#include <axe/pred.h>
#include <axe/box.h>
#include <axe/one.h>
#include <axe/seq.h>
#include <axe/iter.h>
#include <axe/pool.h>
#include <axe/base.h>
#include <axe/error.h>
#include <axe/def.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define ASSERT_ITER_TYPE(_it, _type) ax_assert(ax_one_is(_it->owner, _type), "'%s' is not an iterator of '%s'", #_it, #_type);

void ax_transform(ax_iter first1, ax_iter last1, ax_iter first2, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(&first1, &last1);

	for (ax_iter it1 = first1, it2 = first2; !ax_iter_equal(it1, last1); it1 = ax_iter_next(it1)) {
		ax_pred_do(upred, ax_iter_get(it2), ax_iter_get(it1), NULL);
		it2 = ax_iter_next(it2);
	}
}

ax_bool ax_all_of(ax_iter first, ax_iter last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(&first, &last);

	ax_bool out;
	for (ax_iter it = first; !ax_iter_equal(it, last); it = ax_iter_next(it)) {
		ax_pred_do(upred, &out, ax_iter_get(it), NULL);
		if (!out)
			return ax_false;
	}
	return ax_true;
}

ax_bool ax_any_of(ax_iter first, ax_iter last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(&first, &last);

	ax_bool out;
	for (ax_iter it = first; !ax_iter_equal(it, last); it = ax_iter_next(it)) {
		ax_pred_do(upred, &out, ax_iter_get(it), NULL);
		if (out)
			return ax_true;
	}
	return ax_false;
}

ax_bool ax_none_of(ax_iter first, ax_iter last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(&first, &last);

	ax_bool out;
	for (ax_iter it = first; !ax_iter_equal(it, last); it = ax_iter_next(it)) {
		ax_pred_do(upred, &out, ax_iter_get(it), NULL);
		if (out)
			return ax_false;
	}
	return ax_true;
}

size_t ax_count_if(ax_iter first, ax_iter last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(&first, &last);

	size_t count = 0;
	ax_bool out = ax_false;
	for (ax_iter it = first; !ax_iter_equal(it, last); it = ax_iter_next(it)) {
		ax_pred_do(upred, &out, ax_iter_get(it), NULL);
		count += out ? 1 : 0;
	}
	return count;
}


ax_iter ax_search_of(ax_iter first1, ax_iter last1, ax_iter first2, ax_iter last2)
{
	CHECK_ITER_COMPARABLE(&first1, &last1);

	const ax_seq *seq = first1.owner;
	for (ax_iter it1 = first1; !ax_iter_equal(it1, last1); it1 = ax_iter_next(it1)) {
		for (ax_iter it2 = first2; !ax_iter_equal(it2, last2); it2 =ax_iter_next(it2))
		{
			void *e1 = ax_iter_get(it1);
			void *e2 = ax_iter_get(it2);
			if (seq->elem_tr->equal(e1, e2, seq->elem_tr->size)) {
				return it1; 
			}
		}
	}
	return last1;
}

void ax_generate(ax_iter first, ax_iter last, const void *ptr)
{
	CHECK_ITER_COMPARABLE(&first, &last);

	const ax_seq *seq = first.owner;
	ax_base *base = ax_one_base(ax_cr(seq, seq).one);

	ax_pool *pool = ax_base_pool(base);
	for (ax_iter it = first; !ax_iter_equal(it, last); it = ax_iter_next(it)) {
		void *p = ax_iter_get(it);
		seq->elem_tr->copy(pool, p, ptr, seq->elem_tr->size);
	}
}

void __ax_find_if(ax_iter *first, const ax_iter *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	ax_bool retval;
	while (!ax_iter_equal(*first, *last)) {
		if (ax_pred_do(upred, &retval, ax_iter_get(*first), NULL), retval)
			return;
		*first = ax_iter_next(*first);
	}
}

void __ax_find_if_not(ax_iter *first, const ax_iter *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	ax_bool retval;
	while (!ax_iter_equal(*first, *last)) {
		if (ax_pred_do(upred, &retval, ax_iter_get(*first), NULL), !retval)
			return;
		*first = ax_iter_next(*first);
	}
}


ax_bool ax_sorted(ax_iter first, ax_iter last, const ax_pred *bpred)
{
	CHECK_PARAM_NULL(bpred);
	CHECK_ITER_COMPARABLE(&first, &last);

	if (ax_iter_equal(first, last))
		return ax_true;

	ax_bool retval;
	ax_iter it1 = first, it2 = ax_iter_next(first);
	while (!ax_iter_equal(it2, last)) {
		if (ax_pred_do(bpred, &retval, ax_iter_get(it1), ax_iter_get(it2)), !retval)
			return ax_false;
		it1 = it2;
		it2 = ax_iter_next(it2);
	}
	return ax_true;
}

void __ax_partition(ax_iter *first, const ax_iter *last, const ax_pred *pred)
{
	CHECK_PARAM_NULL(pred);
	CHECK_ITER_COMPARABLE(first, last);
	ax_assert((first->tr->type & AX_IT_FORW) == AX_IT_FORW, "iterator type is not supported");

	*first = ax_find_if_not(*first, *last, pred);
	if (ax_iter_equal(*first, *last))
		return;
	for (ax_iter it = ax_iter_next(*first); !ax_iter_equal(it, *last); it = ax_iter_next(it)) {
		ax_bool retval;
		ax_pred_do(pred, &retval, ax_iter_get(it), NULL);
		if (retval) {
			ax_iter_swap(it, *first);
			*first = ax_iter_next(*first);
		}
	}
}

struct quick_sort_context_st {
	ax_pool *pool;
	const ax_stuff_trait *tr;
	ax_pred pred;
};

static void less_then(ax_bool *out, void *in1, void *in2, const ax_stuff_trait *tr)
{
	*out = tr->less(in1, in2, tr->size);
}

static void quick_sort(struct quick_sort_context_st*ext, const ax_iter *first, const ax_iter *last)
{
	if (ax_iter_equal(*first, *last))
		return;

	ax_iter second = ax_iter_next(*first);

	if (ax_iter_equal(second, *last))
		return;

	ext->pred = ax_pred_binary_make((ax_binary_f) less_then, NULL,
			ax_iter_get(*first), (void*)ext->tr);
	ax_iter right_first = second;
	 __ax_partition(&right_first, last, &ext->pred);

	ax_iter left_last = ax_iter_prev(right_first);
	ext->tr->swap(ax_iter_get(left_last), ax_iter_get(*first), ext->tr->size);

	quick_sort(ext, first, &left_last);
	quick_sort(ext, &right_first, last);
}

ax_fail ax_quick_sort(ax_iter first, ax_iter last)
{
	CHECK_ITER_COMPARABLE(&first, &last);
	ax_assert(ax_one_is(first.owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(&first, AX_IT_BID), "unsupported iterator type");

	const ax_box *box = first.owner;
	ax_one *one = (ax_one*) box;
	ax_base *base = ax_one_base(one);
	ax_pool *pool = ax_base_pool(base);
	const ax_stuff_trait *tr = ax_box_elem_tr(box);

	struct quick_sort_context_st ext = {
		.pool = pool,
		.tr = tr
	};
	quick_sort(&ext, &first, &last);
	return ax_false;
}

void __ax_merge(ax_iter *first1, const ax_iter *last1, ax_iter *first2, const ax_iter *last2, ax_iter *dest)
{
	CHECK_ITER_COMPARABLE(first1, last1);
	CHECK_ITER_COMPARABLE(first2, last2);

	ax_box *box = (ax_box *)first1->owner;
	const ax_stuff_trait *etr = ax_box_elem_tr(box);
	ax_iter src;
	ax_iter src_last;
	while (!ax_iter_equal(*first1, *last1) && !ax_iter_equal(*first2, *last2)) {
		if (etr->less(ax_iter_get(*first1), ax_iter_get(*first2), etr->size)) {
			src = *first1;
			*first1 = ax_iter_next(*first1);
		} else {
			src = *first2;
			*first2 = ax_iter_next(*first2);
		}
		ax_iter_set(*dest, ax_iter_get(src));

		*dest = ax_iter_next(*dest);

	}

	if (!ax_iter_equal(*first1, *last1)) {
		src = *first1;
		src_last = *last1;
	} else {
		src = *first2;
		src_last = *last2;
	}
	while (!ax_iter_equal(src, src_last)) {
		ax_iter_set(*dest, ax_iter_get(src));
		src = ax_iter_next(src);
		*dest = ax_iter_next(*dest);
	}
}

struct merge_sort_context_st {
	const ax_seq *main;
	void **imap;

	const ax_seq *aux;
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


	ax_iter end = ax_box_end(&ext->main->_box);
	ax_iter main_first = { .owner = end.owner, .tr = end.tr };
	ax_iter main_mid  = { .owner = end.owner, .tr = end.tr };
	ax_iter main_last = { .owner = end.owner, .tr = end.tr };
	ax_iter aux_first;
	ax_iter aux_mid;
	ax_iter aux_last;


	main_first.point = ext->imap[left];
	main_mid.point = ext->imap[lmid];
	main_last.point = ext->imap[mid];

	aux_first = ax_seq_at(ext->aux, left);
	aux_mid = ax_merge(main_first, main_mid, main_mid, main_last, aux_first);

	main_first.point = ext->imap[mid];
	main_mid.point = ext->imap[rmid];
	main_last.point = ext->imap[right];

	aux_last = ax_merge(main_first, main_mid, main_mid, main_last, aux_mid);

	main_first.point = ext->imap[left];
	main_last = ax_merge(aux_first, aux_mid, aux_mid, aux_last, main_first);
	
}

ax_fail ax_merge_sort(ax_iter first, ax_iter last)
{
	CHECK_ITER_COMPARABLE(&first, &last);
	ax_assert(ax_one_is(first.owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(&first, AX_IT_FORW), "unsupported iterator type");

	ax_seq_r main_r = { first.owner };

	ax_base *base = ax_one_base(main_r.one);
	const ax_stuff_trait *etr = ax_box_elem_tr(main_r.box);

	size_t size = 0;
	ax_iter cur = ax_box_begin(main_r.box);
	while (!ax_iter_equal(cur, last)) {
		size++;
		cur = ax_iter_next(cur);
	}
	
	void **imap = malloc((size + 1) * sizeof *imap);
	if (!imap) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}
	

	size_t pos = 0;
	cur = ax_box_begin(main_r.box);
	while (!ax_iter_equal(cur, last)) {
		imap[pos++] = cur.point;
		cur = ax_iter_next(cur);
	}
	imap[pos] = cur.point;

	ax_seq *aux = __ax_vector_construct(base, etr);
	if (!aux) {
		free(imap);
		return ax_true;
	}
	ax_seq_trunc(aux, size);

	struct merge_sort_context_st ext = {
		.imap = imap,
		.main = main_r.seq,
		.aux = aux
	};

	merge_sort(0, size, &ext);

	free(imap);
	ax_one_free((ax_one *)aux);
	
	return ax_false;
}

ax_bool ax_equal_to_arr(ax_iter first, ax_iter last, void *arr, size_t size)
{
	CHECK_PARAM_VALIDITY(arr, !!arr == !!size);

	const ax_box *box = first.owner;
	const ax_stuff_trait *tr = ax_box_elem_tr(box);
	ax_assert(size % tr->size == 0, "unexpected size");

	size_t pos = 0;
	while (!ax_iter_equal(first, last) && pos < size) {
		if (!tr->equal(ax_iter_get(first), (ax_byte*)arr + pos, tr->size))
			return ax_false;
		first = ax_iter_next(first);
		pos += tr->size;
	}

	return ax_iter_equal(first, last) == (pos == size);
}

ax_iter ax_binary_search(ax_iter first, ax_iter last, const void* p)
{
	CHECK_ITER_COMPARABLE(&first, &last);
	ax_assert(ax_one_is(first.owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(&first, AX_IT_RAND), "unsupported iterator type");

	void *orignal_last_iter_point = last.point;
	const ax_stuff_trait *tr = ax_box_elem_tr(first.owner);

	ax_iter middle;
	while (!ax_iter_equal(first, last)) {
		long length = ax_iter_dist(first, last);
		long m = length / 2;
		middle = ax_iter_move(first, m);
		if (tr->equal(ax_iter_get(middle), p, tr->size))
			return middle;
		if (tr->less(ax_iter_get(middle), p, tr->size))
			first.point = ax_iter_next(middle).point;
		else
			last.point = middle.point;
	}

	last.point = orignal_last_iter_point;
	return last;
}

ax_iter ax_binary_search_if_not(ax_iter first, ax_iter last, const ax_pred *upred)
{
	CHECK_ITER_COMPARABLE(&first, &last);
	ax_assert(ax_one_is(first.owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(&first, AX_IT_RAND), "unsupported iterator type");

	ax_iter middle;
	while (!ax_iter_equal(first, last)) {
		ax_bool ret;
		long length = ax_iter_dist(first, last);
		long m = length / 2;
		middle = ax_iter_move(first, m);
		if (ax_pred_do(upred, &ret, ax_iter_get(middle), NULL), !ret)
			last.point = middle.point;
		else
			first.point = ax_iter_next(middle).point;
	}

	return first;
}

ax_fail ax_insertion_sort(ax_iter first, ax_iter last)
{
	CHECK_ITER_COMPARABLE(&first, &last);
	ax_assert(ax_one_is(first.owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(&first, AX_IT_FORW), "unsupported iterator type");

	ax_pred pred;
	const ax_stuff_trait* tr = ax_box_elem_tr(first.owner);
	ax_base *base = ax_one_base(first.owner);
	ax_pool *pool = ax_base_pool(base);
	void *tmp = ax_pool_alloc(pool, tr->size);
	if (!tmp) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}

	ax_iter (*search_if_not)(ax_iter first, ax_iter last, const ax_pred *upred)
		= ax_iter_is(&first, AX_IT_RAND)
		? ax_binary_search_if_not
		: ax_find_if_not;

	ax_iter cur = ax_iter_next(first), sorted_first = first, find, swap_prev = { first.owner, first.tr };

	while (!ax_iter_equal(cur, last)) {

		pred = ax_pred_binary_make((ax_binary_f) less_then, NULL,
				ax_iter_get(cur), (void*)tr);
		find = search_if_not(sorted_first, cur, &pred);

		tr->move(tmp, ax_iter_get(cur), tr->size);
		ax_iter swapit = cur;
		while(!ax_iter_equal(find, swapit)) {
			swap_prev.point = ax_iter_prev(swapit).point;
			ax_iter_swap(swapit, swap_prev);
			swapit.point = swap_prev.point;
		}
		tr->move(ax_iter_get(find), tmp, tr->size);

		cur = ax_iter_next(cur);
	}
	ax_pool_free(tmp);
	return ax_false;
}

ax_iter ax_find_first_unsorted(ax_iter first, ax_iter last, const ax_pred *bpred)
{
	CHECK_PARAM_NULL(bpred);
	CHECK_ITER_COMPARABLE(&first, &last);

	if (ax_iter_equal(first, last))
		return last;

	ax_bool retval;
	ax_iter it1 = first, it2 = ax_iter_next(first);
	while (!ax_iter_equal(it2, last)) {
		if (ax_pred_do(bpred, &retval, ax_iter_get(it1), ax_iter_get(it2)), !retval)
			return it2;
		it1.point = it2.point;
		it2.point = ax_iter_next(it2).point;
	}
	return last;

}
