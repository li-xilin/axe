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
#include "seq.h"

#ifndef AX_DUMP_DEFINED
#define AX_DUMP_DEFINED
typedef struct ax_dump_st ax_dump;
#endif

#ifndef AX_TRIE_DEFINED
#define AX_TRIE_DEFINED
typedef struct ax_trie_st ax_trie;
#endif

#define ax_baseof_ax_trie ax_box

ax_abstract_code_begin(ax_trie)
	const ax_iter_trait iter_root;
	void       *(*put)      (ax_trie *trie, const ax_seq *key, const void *val, va_list *ap);
	void       *(*get)      (const ax_trie *trie, const ax_seq *key);
	ax_iter     (*at)       (const ax_trie *trie, const ax_seq *key);
	bool        (*exist)    (const ax_trie *trie, const ax_seq *key, bool *valued);
	bool        (*erase)    (ax_trie *trie, const ax_seq *key);
	bool        (*prune)    (ax_trie *trie, const ax_seq *key);
	ax_fail     (*rekey)    (ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to);
	const void *(*it_word)  (const ax_citer *it);
	ax_iter     (*it_begin) (const ax_citer *it);
	ax_iter     (*it_end)   (const ax_citer *it);
	bool        (*it_parent)(const ax_citer *it, ax_iter *parent);
	bool        (*it_valued)(const ax_citer *it);
	void        (*it_clean) (const ax_iter *it);
ax_end;

ax_abstract_data_begin(ax_trie)
	const ax_trait *key_tr;
ax_end;

ax_abstract_declare(3, ax_trie);

inline static void *ax_trie_put(ax_trie *trie, const ax_seq *key, const void *val)
{
	return ax_obj_do(trie, put, key, val, NULL);
}

inline static void *ax_trie_iput(ax_trie *trie, const ax_seq *key, ...)
{
	va_list ap;
	va_start(ap, key);
	void *ret = ax_obj_do(trie, put, key, NULL, &ap);
	va_end(ap);
	return ret;
}

inline static bool ax_trie_erase(ax_trie *trie, const ax_seq *key)
{
	return ax_obj_do(trie, erase, key);
}

inline static bool ax_trie_prune(ax_trie *trie, const ax_seq *key)
{
	return ax_obj_do(trie, prune, key);
}

inline static void *ax_trie_get(ax_trie *trie, const ax_seq *key)
{
	return ax_obj_do(trie, get, key);
}

inline static ax_iter ax_trie_at(ax_trie *trie, const ax_seq *key)
{
	return ax_obj_do(trie, at, key);
}

inline static ax_citer ax_trie_cat(const ax_trie *trie, const ax_seq *key)
{
	ax_iter it = ax_obj_do(trie, at, key);
	void *p = &it;
	return *(ax_citer *)p;
}

inline static void *ax_trie_cget(const ax_trie *trie, const ax_seq *key)
{
	return ax_obj_do(trie, get, key);
}

inline static bool ax_trie_exist(const ax_trie *trie, const ax_seq *key, bool *valued)
{
	return ax_obj_do(trie, exist, key, valued);
}

inline static const void *ax_trie_iter_word(const ax_iter *it)
{
	return ax_class_trait((ax_trie *)ax_iter_box(it)).it_word(ax_iter_cc(it));
}

inline static const void *ax_trie_citer_word(const ax_citer *it)
{
	return ax_class_trait((ax_trie *)ax_citer_box(it)).it_word(it);
}

inline static ax_iter ax_trie_iter_begin(const ax_iter *it)
{
	return ax_class_trait((ax_trie *)ax_iter_box(it)).it_begin(ax_iter_cc(it));
}

inline static ax_iter ax_trie_citer_begin(const ax_citer *it)
{
	return ax_class_trait((ax_trie *)ax_citer_box(it)).it_begin(it);
}

inline static ax_citer ax_trie_citer_cbegin(const ax_citer *it)
{
	return ax_iter_citer(ax_class_trait((ax_trie *)ax_citer_box(it)).it_begin(it));
}

inline static ax_iter ax_trie_iter_end(const ax_iter *it)
{
	return ax_class_trait((ax_trie *)ax_iter_box(it)).it_end(ax_iter_cc(it));
}

inline static ax_iter ax_trie_citer_end(const ax_citer *it)
{
	return ax_class_trait((ax_trie *)ax_citer_box(it)).it_end(it);
}

inline static ax_citer ax_trie_citer_cend(const ax_citer *it)
{
	return ax_iter_citer(ax_class_trait((ax_trie *)ax_citer_box(it)).it_end(it));
}

inline static ax_fail ax_trie_rekey(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to)
{
	return ax_obj_do(trie, rekey, key_from, key_to);
}

inline static bool ax_trie_iter_valued(const ax_iter *it)
{
	return ax_class_trait((ax_trie *)ax_iter_box(it)).it_valued(ax_iter_cc(it));
}

inline static bool ax_trie_citer_valued(const ax_citer *it)
{
	return ax_class_trait((ax_trie *)ax_citer_box(it)).it_valued(it);
}

inline static const ax_trait *ax_trie_key_tr(const ax_trie *trie)
{
	return ax_class_data(trie).key_tr;
}

typedef bool (*ax_trie_enum_cb_f)(const ax_trie *trie, const ax_seq *key, const void *val, void *ctx);

ax_fail ax_trie_enum(const ax_trie *trie, ax_trie_enum_cb_f cb, void *ctx);

ax_dump *ax_trie_dump(const ax_trie *trie);

#endif
