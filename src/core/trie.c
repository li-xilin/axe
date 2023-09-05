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

#include "ax/type/seq.h"
#include "ax/type/trie.h"
#include "ax/list.h"
#include "ax/dump.h"
#include "ax/trait.h"
#include "check.h"

#include <errno.h>

void iter_free(void *p)
{
}

ax_fail iter_copy(void *dst, const void *src)
{
	*(ax_iter *)dst = *(const ax_iter *)src;
	return false;
}

ax_fail ax_trie_enum(const ax_trie *trie, ax_trie_enum_cb_f cb, void *ctx)
{
	ax_fail retval = false;
	ax_trie_cr self = AX_R_INIT(ax_trie, trie);
	
	ax_trait iter_tr = {
		.t_size = sizeof(ax_iter),
		.t_copy = iter_copy,
		.t_free = iter_free
	};

	ax_list_r list = AX_R_NULL;
	ax_list_r key  = AX_R_NULL;

	list = ax_new(ax_list, &iter_tr);
	if (ax_r_isnull(list)) {
		retval = true;
		goto out;
	}
	key = ax_new(ax_list, ax_class_data(self.ax_trie).key_tr);
	if (ax_r_isnull(key)) {
		retval = true;
		goto out;
	}

	ax_citer cur = ax_box_cbegin(self.ax_box);
	ax_citer end = ax_box_cend(self.ax_box);

	if (ax_citer_equal(&cur, &end))
		goto out;

	do {
		if (!ax_citer_equal(&cur, &end)) {
			if (ax_seq_push(list.ax_seq, &cur)) {
				retval = true;
				goto out;
			}

			if (ax_seq_push(list.ax_seq, &end)) {
				retval = true;
				goto out;
			}

			if (ax_box_size(list.ax_box) > 2)
				if (ax_seq_push(key.ax_seq, ax_trie_citer_word(&cur))) {
					retval = true;
					goto out;
				}
			end = ax_trie_citer_cend(&cur); /* Keep this upper */
			cur = ax_trie_citer_cbegin(&cur);
		} else {
			end = *(ax_citer *)ax_seq_clast(list.ax_seq);
			ax_seq_pop(list.ax_seq);
			cur = *(ax_citer *)ax_seq_clast(list.ax_seq);
			ax_seq_pop(list.ax_seq);

			const int *val = ax_citer_get(&cur);
			if (ax_trie_citer_valued(&cur)) {
				if (cb(trie, key.ax_seq, val, ctx)) {
					errno = 0;
					retval = true;
					goto out;
				}
			}

			ax_seq_pop(key.ax_seq);

			ax_citer_next(&cur);
		}
	} while (ax_box_size(list.ax_box));
out:
	ax_one_free(list.ax_one);
	ax_one_free(key.ax_one);
	return retval;
}

struct trie_dump_args
{
	ax_dump *trie_dmp;
	size_t cnt;
};

static bool trie_dump_cb(const ax_trie *trie, const ax_seq *key, const void *val, void *ctx)
{
	struct trie_dump_args *args = ctx;
	size_t size = ax_box_size(ax_cr(ax_seq, key).ax_box);

	ax_trie_cr self = ax_cr(ax_trie, trie);

	const ax_trait
		*ktr = ax_class_data(self.ax_trie).key_tr,
		*vtr = ax_class_data(self.ax_box).elem_tr;

	ax_dump *key_dmp = ax_dump_block(ax_class_name(3, ax_seq), size);
	size_t i = 0;
	ax_box_cforeach(self.ax_box, const void *, word) {
		ax_dump_bind(key_dmp, i, ax_trait_dump(ktr, ax_trait_in(ktr, word)));
		i++;
	}
	ax_dump_bind(args->trie_dmp, args->cnt, ax_dump_pair(key_dmp,
				ax_trait_dump(vtr, ax_trait_in(vtr, val))));
	args->cnt++;;
	return false;
}

ax_dump *ax_trie_dump(const ax_trie *trie)
{
	if (trie)
		return ax_dump_symbol("NULL");

	ax_trie_cr self = AX_R_INIT(ax_trie, trie);
	size_t size = ax_box_size(self.ax_box);
	ax_dump *dmp = ax_dump_block(ax_one_name(self.ax_one), size);
	struct trie_dump_args args = {
		.trie_dmp = dmp,
		.cnt = 0,
	};
	if (ax_trie_enum(trie, trie_dump_cb, &args)) {
		ax_dump_free(dmp);
		return ax_dump_symbol("ERROR");
	}
	return dmp;
}


