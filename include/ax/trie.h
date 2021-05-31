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

#ifndef AX_TRIE_H
#define AX_TRIE_H
#include "box.h"
#include "seq.h"
#include "def.h"

#define AX_TRIE_NAME AX_BOX_NAME ".trie"

#ifndef AX_TRIE_DEFINED
#define AX_TRIE_DEFINED
typedef struct ax_trie_st ax_trie;
#endif

#ifndef AX_TRIE_TRAIT_DEFINED
#define AX_TRIE_TRAIT_DEFINED
typedef struct ax_trie_trait_st ax_trie_trait;
#endif

typedef void *(*ax_trie_put_f)  (ax_trie *trie, const ax_seq *key, const void *val);
typedef void *(*ax_trie_get_f)  (const ax_trie *trie, const ax_seq *key);
typedef ax_iter (*ax_trie_at_f)   (const ax_trie *trie, const ax_seq *key);
typedef bool (*ax_trie_exist_f)(const ax_trie *trie, const ax_seq *key, bool *valued);
typedef bool (*ax_trie_erase_f)(ax_trie *trie, const ax_seq *key);
typedef bool (*ax_trie_prune_f)(ax_trie *trie, const ax_seq *key);
typedef ax_fail (*ax_trie_rekey_f)(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to);

typedef const void *(*ax_trie_it_word_f)(const ax_citer *it);
typedef ax_iter (*ax_trie_it_begin_f)(const ax_citer *it);
typedef ax_iter (*ax_trie_it_end_f)(const ax_citer *it);
typedef bool (*ax_trie_it_parent_f)(const ax_citer *it, ax_iter *parent);
typedef bool (*ax_trie_it_valued_f)(const ax_citer *it);
typedef void (*ax_trie_it_clean_f)(const ax_iter *it);


struct ax_trie_trait_st
{
	const ax_box_trait box;
	const ax_iter_trait iter_root;

	const ax_trie_put_f put;
	const ax_trie_get_f get;
	const ax_trie_at_f at;
	const ax_trie_exist_f exist;
	const ax_trie_erase_f erase;
	const ax_trie_prune_f prune;
	const ax_trie_rekey_f rekey;
	
	const ax_trie_it_word_f it_word;
	const ax_trie_it_begin_f it_begin;
	const ax_trie_it_end_f it_end;
	const ax_trie_it_parent_f it_parent;
	const ax_trie_it_valued_f it_valued;
	const ax_trie_it_clean_f clean;
};

typedef struct ax_trie_env_st
{
	ax_one_env one;
	const ax_stuff_trait *key_tr;
	const ax_stuff_trait *val_tr;
} ax_trie_env;

struct ax_trie_st
{
	const ax_trie_trait *const tr;
	ax_trie_env env;
};

typedef union
{
	const ax_trie *trie;
	const ax_box *box;
	const ax_any *any;
	const ax_one *one;
} ax_trie_cr;

typedef union
{
	ax_trie *trie;
	ax_box *box;
	ax_any *any;
	ax_one *one;
	ax_trie_cr c;
} ax_trie_r;

inline static void *ax_trie_put(ax_trie *trie, const ax_seq *key, const void *val)
{
	ax_trait_require(trie, trie->tr->put);
	return trie->tr->put(trie, key, val);
}

inline static bool ax_trie_erase(ax_trie *trie, const ax_seq *key)
{
	ax_trait_require(trie, trie->tr->erase);
	return trie->tr->erase(trie, key);
}

inline static bool ax_trie_prune(ax_trie *trie, const ax_seq *key)
{
	ax_trait_require(trie, trie->tr->prune);
	return trie->tr->prune(trie, key);
}

inline static void *ax_trie_get(ax_trie *trie, const ax_seq *key)
{
	ax_trait_require(trie, trie->tr->get);
	return trie->tr->get(trie, key);
}

inline static ax_iter ax_trie_at(ax_trie *trie, const ax_seq *key)
{
	ax_trait_require(trie, trie->tr->at);
	return trie->tr->at(trie, key);
}

inline static ax_citer ax_trie_cat(const ax_trie *trie, const ax_seq *key)
{
	ax_trait_require(trie, trie->tr->at);
	ax_iter it = trie->tr->at(trie, key);
	void *p = &it;
	return *(ax_citer *)p;
}

inline static void *ax_trie_cget(const ax_trie *trie, const ax_seq *key)
{
	ax_trait_require(trie, trie->tr->get);
	return trie->tr->get(trie, key);
}

inline static bool ax_trie_exist(const ax_trie *trie, const ax_seq *key, bool *valued)
{
	ax_trait_require(trie, trie->tr->exist);
	return trie->tr->exist(trie, key, valued);
}

inline static const void *ax_trie_iter_word(const ax_iter *it)
{
	ax_trie_cr self_r = { .box = ax_iter_box(it) };
	return self_r.trie->tr->it_word(ax_iter_c(it));
}

inline static const void *ax_trie_citer_word(const ax_citer *it)
{
	ax_trie_cr self_r = { .box = ax_citer_box(it) };
	return self_r.trie->tr->it_word(it);
}

inline static ax_iter ax_trie_iter_begin(const ax_iter *it)
{
	ax_trie_cr self_r = { .box = ax_iter_box(it) };
	return self_r.trie->tr->it_begin(ax_iter_c(it));
}

inline static ax_iter ax_trie_citer_begin(const ax_citer *it)
{
	ax_trie_cr self_r = { .box = ax_citer_box(it) };
	return self_r.trie->tr->it_begin(it);
}

inline static ax_iter ax_trie_iter_end(const ax_iter *it)
{
	ax_trie_cr self_r = { .box = ax_iter_box(it) };
	return self_r.trie->tr->it_end(ax_iter_c(it));
}

inline static ax_iter ax_trie_citer_end(const ax_citer *it)
{
	ax_trie_cr self_r = { .box = ax_citer_box(it) };
	return self_r.trie->tr->it_end(it);
}

inline static ax_fail ax_trie_rekey(ax_trie *trie,
		const ax_seq *key_from, const ax_seq *key_to)
{
	ax_trait_require(trie, trie->tr->rekey);
	return trie->tr->rekey(trie, key_from, key_to);
}

inline static bool ax_trie_iter_valued(const ax_iter *it)
{
	ax_trie_cr self_r = { .box = ax_iter_box(it) };
	return self_r.trie->tr->it_valued(ax_iter_c(it));
}

inline static bool ax_trie_citer_valued(const ax_citer *it)
{
	ax_trie_cr self_r = { .box = ax_citer_box(it) };
	return self_r.trie->tr->it_valued(it);
}

/*
inline static bool ax_trie_iter_top(const ax_citer *it)
{
	ax_trie_cr self_r = { .box = ax_citer_box(it) };
	return self_r.trie->tr->valued(it);
}
*/

inline static const ax_stuff_trait *ax_trie_key_tr(const ax_trie *trie)
{
	ax_trie_cr self_r = { .trie = trie };
	return self_r.trie->env.key_tr;
}

typedef bool (*ax_trie_enum_cb_f)(ax_trie *trie, const ax_seq *key, const void *val, void *ctx);

ax_fail ax_trie_enum(ax_trie *trie, ax_trie_enum_cb_f cb, void *ctx);

#endif
