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

#include <axe/trie.h>
#include <axe/list.h>
#include <axe/base.h>

ax_fail ax_trie_enum(ax_trie *trie, ax_trie_enum_cb_f cb, void *ctx)
{
	ax_fail retval = ax_false;
	ax_trie_r self_r = { trie };
	ax_base *base = ax_one_base(self_r.one);
	
	ax_stuff_trait iter_tr = {
		.size = sizeof(ax_iter),
		.copy = ax_stuff_mem_copy,
		.init = ax_stuff_mem_init,
		.free = ax_stuff_mem_free
	};

	ax_list_r list_r = { NULL };
	ax_list_r key_r  = { NULL };

	list_r = ax_list_create(ax_base_local(base), &iter_tr);
	if (!list_r.one) {
		retval = ax_true;
		goto out;
	}
	key_r = ax_list_create(ax_base_local(base), ax_box_elem_tr(self_r.box));
	if (!key_r.one) {
		retval = ax_true;
		goto out;
	}

	ax_iter cur = ax_box_begin(self_r.box);
	ax_iter end = ax_box_end(self_r.box);
	do {
		if (!ax_iter_equal(&cur, &end)) {
			if (ax_seq_push(list_r.seq, &cur)) {
				retval = ax_true;
				goto out;
			}

			if (ax_seq_push(list_r.seq, &end)) {
				retval = ax_true;
				goto out;
			}

			if (ax_box_size(list_r.box) > 2)
				if (ax_seq_push(key_r.seq, ax_trie_iter_word(&cur))) {
					retval = ax_true;
					goto out;
				}
			end = ax_trie_iter_end(&cur); /* Keep this upper */
			cur = ax_trie_iter_begin(&cur);
		} else {
			end = *(ax_iter *)ax_seq_last(list_r.seq);
			ax_seq_pop(list_r.seq);
			cur = *(ax_iter *)ax_seq_last(list_r.seq);
			ax_seq_pop(list_r.seq);

			int *val = ax_iter_get(&cur);
			if (ax_trie_iter_valued(&cur)) {
				if (cb(trie, key_r.seq, val, ctx))
					goto out;
			}

			ax_seq_pop(key_r.seq);

			ax_iter_next(&cur);
		}
	} while (ax_box_size(list_r.box));
out:
	ax_one_free(list_r.one);
	ax_one_free(key_r.one);
	return retval;
}
