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

#include <ax/trie.h>
#include <ax/list.h>
#include <ax/dump.h>

#include <errno.h>

ax_fail ax_trie_enum(const ax_trie *trie, ax_trie_enum_cb_f cb, void *ctx)
{
	ax_fail retval = false;
	ax_trie_cr self_r = { trie };
	
	ax_stuff_trait iter_tr = {
		.size = sizeof(ax_iter),
		.copy = ax_stuff_mem_copy,
		.init = ax_stuff_mem_init,
		.free = ax_stuff_mem_free
	};

	ax_list_r list_r = { NULL };
	ax_list_r key_r  = { NULL };

	list_r.seq = __ax_list_construct(&iter_tr);
	if (!list_r.one) {
		retval = true;
		goto out;
	}
	key_r.seq = __ax_list_construct(self_r.trie->env.key_tr);
	if (!key_r.one) {
		retval = true;
		goto out;
	}

	ax_citer cur = ax_box_cbegin(self_r.box);
	ax_citer end = ax_box_cend(self_r.box);

	if (ax_citer_equal(&cur, &end))
		goto out;

	do {
		if (!ax_citer_equal(&cur, &end)) {
			if (ax_seq_push(list_r.seq, &cur)) {
				retval = true;
				goto out;
			}

			if (ax_seq_push(list_r.seq, &end)) {
				retval = true;
				goto out;
			}

			if (ax_box_size(list_r.box) > 2)
				if (ax_seq_push(key_r.seq, ax_trie_citer_word(&cur))) {
					retval = true;
					goto out;
				}
			end = ax_trie_citer_cend(&cur); /* Keep this upper */
			cur = ax_trie_citer_cbegin(&cur);
		} else {
			end = *(ax_citer *)ax_seq_clast(list_r.seq);
			ax_seq_pop(list_r.seq);
			cur = *(ax_citer *)ax_seq_clast(list_r.seq);
			ax_seq_pop(list_r.seq);

			const int *val = ax_citer_get(&cur);
			if (ax_trie_citer_valued(&cur)) {
				if (cb(trie, key_r.seq, val, ctx)) {
					errno = 0;
					retval = true;
					goto out;
				}
			}

			ax_seq_pop(key_r.seq);

			ax_citer_next(&cur);
		}
	} while (ax_box_size(list_r.box));
out:
	ax_one_free(list_r.one);
	ax_one_free(key_r.one);
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
	size_t size = ax_box_size(ax_cr(seq, key).box);
	ax_dump *pair_dmp = NULL, *key_dmp = NULL, *val_dmp = NULL;
	pair_dmp = ax_dump_pair();
	if (!pair_dmp)
		goto fail;
	key_dmp = ax_dump_block(AX_SEQ_NAME, size);
	if (!key_dmp) {
		goto fail;
	}

	const ax_stuff_trait *ktr = trie->env.key_tr,
	      *vtr = ax_box_elem_tr(ax_cr(trie, trie).box);

	val_dmp = vtr->dump(vtr->link ? &val : val, vtr->size);
	if (!val_dmp)
		goto fail;

	size_t i = 0;
	ax_box_cforeach(ax_cr(seq, key).box, const void *, word) {
		ax_dump *word_dmp = ktr->dump(ktr->link ? &word : word, ktr->size);
		if (!word_dmp)
			goto fail;
		ax_dump_bind(key_dmp, i, word_dmp);
		i++;
	}

	ax_dump_bind(pair_dmp, 0, key_dmp);
	ax_dump_bind(pair_dmp, 1, val_dmp);
	ax_dump_bind(args->trie_dmp, args->cnt, pair_dmp);
	args->cnt++;;
	return false;
fail:
	ax_dump_free(pair_dmp);
	ax_dump_free(key_dmp);
	ax_dump_free(val_dmp);
	return true;
}

ax_dump *ax_trie_dump(const ax_trie *trie)
{
	ax_trie_cr self = { .trie = trie };
	size_t size = ax_box_size(self.box);
	ax_dump *dmp = ax_dump_block(ax_one_name(self.one), size);
	struct trie_dump_args args = {
		.trie_dmp = dmp,
		.cnt = 0,
	};
	if (ax_trie_enum(trie, trie_dump_cb, &args))
		return NULL;
	return dmp;
}


