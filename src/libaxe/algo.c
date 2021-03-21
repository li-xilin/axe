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

ax_bool ax_all_of(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	ax_bool out;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		if (!out)
			return ax_false;
	}
	return ax_true;
}

ax_bool ax_any_of(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	ax_bool out;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		if (out)
			return ax_true;
	}
	return ax_false;
}

ax_bool ax_none_of(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	ax_bool out;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		if (out)
			return ax_false;
	}
	return ax_true;
}

size_t ax_count_if(const ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	size_t count = 0;
	ax_bool out = ax_false;
	for (ax_citer it = *first; !ax_citer_equal(&it, last); ax_citer_next(&it)) {
		ax_pred_do(upred, &out, ax_citer_get(&it), NULL);
		count += out ? 1 : 0;
	}
	return count;
}

ax_iter ax_search_of(const ax_iter *first1, const ax_iter *last1, const ax_citer *first2, const ax_citer *last2)
{
	CHECK_ITER_COMPARABLE(first1, last1);

	ax_seq *seq = first1->owner;
	for (ax_iter it1 = *first1; !ax_iter_equal(&it1, last1); ax_iter_next(&it1)) {
		for (ax_citer it2 = *first2; !ax_citer_equal(&it2, last2); ax_citer_next(&it2))
		{
			void *e1 = ax_iter_get(&it1);
			const void *e2 = ax_citer_get(&it2);
			if (seq->elem_tr->equal(e1, e2, seq->elem_tr->size)) {
				return it1; 
			}
		}
	}
	return *last1;
}

void ax_generate(const ax_iter *first, const ax_iter *last, const void *ptr)
{
	CHECK_ITER_COMPARABLE(first, last);

	ax_seq *seq = first->owner;
	ax_base *base = ax_one_base(ax_cr(seq, seq).one);
	ax_pool *pool = ax_base_pool(base);

	for (ax_iter it = *first; !ax_iter_equal(&it, last); ax_iter_next(&it)) {
		void *p = ax_iter_get(&it);
		seq->elem_tr->copy(pool, p, ptr, seq->elem_tr->size);
	}
}

void ax_find_if(ax_citer *first, const ax_citer *last, const ax_pred *upred)
{
	CHECK_PARAM_NULL(upred);
	CHECK_ITER_COMPARABLE(first, last);

	ax_bool retval;
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

	ax_bool retval;
	while (!ax_citer_equal(first, last)) {
		if (ax_pred_do(upred, &retval, ax_citer_get(first), NULL), !retval)
			return;
		ax_citer_next(first);
	}
}

ax_bool ax_sorted(const ax_citer *first, const ax_citer *last, const ax_pred *bpred)
{
	CHECK_PARAM_NULL(bpred);
	CHECK_ITER_COMPARABLE(first, last);

	if (ax_citer_equal(first, last))
		return ax_true;

	ax_bool retval;
	ax_citer it1 = *first, it2 = it1;
	ax_citer_next(&it2);

	while (!ax_citer_equal(&it2, last)) {
		if (ax_pred_do(bpred, &retval, ax_citer_get(&it1), ax_citer_get(&it2)), !retval)
			return ax_false;
		it1 = it2;
		ax_citer_next(&it2);
	}
	return ax_true;
}

void ax_partition(ax_iter *first, const ax_iter *last, const ax_pred *pred)
{
	CHECK_PARAM_NULL(pred);
	CHECK_ITER_COMPARABLE(first, last);
	ax_assert((first->tr->ctr.type & AX_IT_FORW) == AX_IT_FORW,
			"iterator type is not supported");

	ax_find_if_not(ax_iter_c(first), ax_iter_c(last), pred);

	if (ax_iter_equal(first, last))
		return;

	ax_iter it = *first;
	for (ax_iter_next(&it); !ax_iter_equal(&it, last); ax_iter_next(&it)) {
		ax_bool retval;
		ax_pred_do(pred, &retval, ax_iter_get(&it), NULL);
		if (retval) {
			ax_iter_swap(&it, first);
			ax_iter_next(first);
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
	if (ax_iter_equal(first, last))
		return;

	ax_iter second = *first;
	ax_iter_next(&second);

	if (ax_iter_equal(&second, last))
		return;

	ext->pred = ax_pred_binary_make((ax_binary_f) less_then, NULL,
			ax_iter_get(first), (void*)ext->tr);
	ax_iter right_first = second;
	 ax_partition(&right_first, last, &ext->pred);

	ax_iter left_last = right_first;
	ax_iter_prev(&left_last);
	ext->tr->swap(ax_iter_get(&left_last), ax_iter_get(first), ext->tr->size);

	quick_sort(ext, first, &left_last);
	quick_sort(ext, &right_first, last);
}

ax_fail ax_quick_sort(const ax_iter *first, const ax_iter *last)
{
	CHECK_ITER_COMPARABLE(first, last);
	ax_assert(ax_one_is(first->owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(first, AX_IT_BID), "unsupported iterator type");

	const ax_box *box = first->owner;
	ax_one *one = (ax_one*) box;
	ax_base *base = ax_one_base(one);
	ax_pool *pool = ax_base_pool(base);
	const ax_stuff_trait *tr = ax_box_elem_tr(box);

	struct quick_sort_context_st ext = {
		.pool = pool,
		.tr = tr
	};
	quick_sort(&ext, first, last);
	return ax_false;
}

void ax_merge(const ax_citer *first1, const ax_citer *last1, const ax_citer *first2, const ax_citer *last2, ax_iter *dest)
{
	CHECK_ITER_COMPARABLE(first1, last1);
	CHECK_ITER_COMPARABLE(first2, last2);

	ax_box *box = (ax_box *)first1->owner;
	const ax_stuff_trait *etr = ax_box_elem_tr(box);
	ax_citer cur1 = *first1, cur2 = *first2;

	ax_citer src;
	ax_citer src_last;
	while (!ax_citer_equal(&cur1, last1) && !ax_citer_equal(&cur2, last2)) {
		if (etr->less(ax_citer_get(&cur1), ax_citer_get(&cur2), etr->size)) {
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
	const ax_seq *main;
	void **imap;

	ax_seq *aux;
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
	CHECK_ITER_COMPARABLE(first, last);
	ax_assert(ax_one_is(first->owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(first, AX_IT_FORW), "unsupported iterator type");

	ax_seq_r main_r = { first->owner };

	ax_base *base = ax_one_base(main_r.one);
	const ax_stuff_trait *etr = ax_box_elem_tr(main_r.box);

	size_t size = 0;
	ax_iter cur = ax_box_begin(main_r.box);
	while (!ax_iter_equal(&cur, last)) {
		size++;
		ax_iter_next(&cur);
	}
	
	void **imap = malloc((size + 1) * sizeof *imap);
	if (!imap) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}
	
	size_t pos = 0;
	cur = ax_box_begin(main_r.box);
	while (!ax_iter_equal(&cur, last)) {
		imap[pos++] = cur.point;
		ax_iter_next(&cur);
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

ax_bool ax_equal_to_arr(const ax_iter *first, const ax_iter *last, void *arr, size_t size)
{
	CHECK_PARAM_VALIDITY(arr, !!arr == !!size);

	const ax_box *box = first->owner;
	const ax_stuff_trait *tr = ax_box_elem_tr(box);
	ax_assert(size % tr->size == 0, "unexpected size");

	size_t pos = 0;
	ax_iter cur = *first;
	while (!ax_iter_equal(&cur, last) && pos < size) {
		if (!tr->equal(ax_iter_get(&cur), (ax_byte*)arr + pos, tr->size))
			return ax_false;
		ax_iter_next(&cur);
		pos += tr->size;
	}
	return ax_iter_equal(&cur, last) == (pos == size);
}

void ax_binary_search(ax_citer *first, const ax_citer *last, const void* p)
{
	CHECK_ITER_COMPARABLE(first, last);
	ax_assert(ax_one_is(first->owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_citer_is(first, AX_IT_RAND), "unsupported citerator type");

	void *orignal_last_citer_point = last->point;
	const ax_stuff_trait *tr = ax_box_elem_tr(first->owner);

	ax_citer left = *first, right = *last, middle;
	while (!ax_citer_equal(&left, &right)) {
		long length = ax_citer_dist(&left, &right);
		long m = length / 2;
		middle = left;
		ax_citer_move(&middle, m);
		if (tr->equal(ax_citer_get(&middle), p, tr->size)) {
			first->point = middle.point;
			return;
		}

		if (tr->less(ax_citer_get(&middle), p, tr->size)) {
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
	ax_assert(ax_one_is(first->owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_citer_is(first, AX_IT_RAND), "unsupported citerator type");

	ax_citer left = *first, right = *last, middle = { .owner = first->owner, .tr = first->tr };
	while (!ax_citer_equal(&left, &right)) {
		ax_bool ret;
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
	ax_assert(ax_one_is(first->owner, "one.any.box.seq"), "unsupported container type");
	ax_assert(ax_iter_is(first, AX_IT_FORW), "unsupported iterator type");

	ax_pred pred;
	const ax_stuff_trait* tr = ax_box_elem_tr(first->owner);
	ax_base *base = ax_one_base(first->owner);
	ax_pool *pool = ax_base_pool(base);
	void *tmp = ax_pool_alloc(pool, tr->size);
	if (!tmp) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return ax_true;
	}

	void (*search_if_not)(ax_citer *first, const ax_citer *last, const ax_pred *upred)
		= ax_iter_is(first, AX_IT_RAND)
		? ax_binary_search_if_not
		: ax_find_if_not;


	ax_iter cur = *first, sorted_first = *first, swap_prev = { first->owner, first->tr }, find;
	ax_iter_next(&cur);

	while (!ax_iter_equal(&cur, last)) {

		pred = ax_pred_binary_make((ax_binary_f) less_then, NULL,
				ax_iter_get(&cur), (void*)tr);
		find = sorted_first;
		search_if_not(ax_iter_c(&find), ax_iter_c(&cur), &pred);

		tr->move(tmp, ax_iter_get(&cur), tr->size);
		ax_iter swapit = cur;
		while(!ax_iter_equal(&find, &swapit)) {
			swap_prev.point = swapit.point;
			ax_iter_prev(&swap_prev);
			ax_iter_swap(&swapit, &swap_prev);
			swapit.point = swap_prev.point;
		}
		tr->move(ax_iter_get(&find), tmp, tr->size);

		ax_iter_next(&cur);
	}
	ax_pool_free(tmp);
	return ax_false;
}

void ax_find_first_unsorted(ax_citer *first, const ax_citer *last, const ax_pred *bpred)
{
	CHECK_PARAM_NULL(bpred);
	CHECK_ITER_COMPARABLE(first, last);

	if (ax_citer_equal(first, last)) {
		first->point = last->point;
		return;
	}

	ax_bool retval;
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
