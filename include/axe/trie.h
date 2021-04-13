/*
  *Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
 *
  *Permission is hereby granted, free of charge, to any person obtaining a copy
  *of map software and associated documentation files (the "Software"), to deal
  *in the Software without restriction, including without limitation the rights
  *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  *copies of the Software, and to permit persons to whom the Software is
  *furnished to do so, subject to the following conditions:
 *
  *The above copyright notice and map permission notice shall be included in
  *all copies or substantial portions of the Software.
 *
  *THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  *IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  *FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  *AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  *LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  *OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  *THE SOFTWARE.
 */

#ifndef AXE_TRIE_H_
#define AXE_TRIE_H_
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

typedef ax_fail (*ax_trie_put_f)  (ax_trie *trie, const ax_seq *key, const void *val);
typedef void   *(*ax_trie_get_f)  (const ax_trie *trie, const ax_seq *key);
typedef ax_iter (*ax_trie_at_f)  (const ax_trie *trie, const ax_seq *key);
typedef ax_bool (*ax_trie_exist_f)(const ax_trie *trie, const ax_seq *key);
typedef void    (*ax_trie_erase_f)(ax_trie *trie, const ax_seq *key);
typedef void    (*ax_trie_rekey_f)(ax_trie *trie, const ax_seq *key_from, const ax_seq *key_to);
typedef ax_iter (*ax_trie_root_f)(ax_trie *trie);

typedef void    (*ax_trie_it_key_f)(const ax_citer *it, ax_seq *key);
typedef ax_iter (*ax_trie_it_begin_f)(const ax_citer *it);
typedef ax_iter (*ax_trie_it_end_f)(const ax_citer *it);
typedef ax_iter (*ax_trie_it_parent_f)(const ax_citer *it);


struct ax_trie_trait_st
{
	const ax_box_trait box;

	const ax_trie_put_f put;
	const ax_trie_get_f get;
	const ax_trie_at_f at;
	const ax_trie_exist_f exist;
	const ax_trie_erase_f erase;
	const ax_trie_rekey_f rekey;
	const ax_trie_root_f root;
	
	const ax_trie_it_key_f it_key;
	const ax_trie_it_begin_f it_begin;
	const ax_trie_it_end_f it_end;
	const ax_trie_it_parent_f it_parent;
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

inline static ax_fail ax_trie_put(ax_trie *trie, const void *key, const void *val)
{
	ax_trait_require(trie, trie->tr->put);
	return trie->tr->put(trie, key, val);
}

inline static void ax_trie_erase(ax_trie *trie, const void *key)
{
	ax_trait_require(trie, trie->tr->erase);
	trie->tr->erase(trie, key);
}

inline static void *ax_trie_get(ax_trie *trie, const void *key)
{
	ax_trait_require(trie, trie->tr->get);
	return trie->tr->get(trie, key);
}

inline static void *ax_trie_cget(const ax_trie *trie, const void *key)
{
	ax_trait_require(trie, trie->tr->get);
	return trie->tr->get(trie, key);
}

inline static ax_bool ax_trie_exist(const ax_trie *trie, const void *key)
{
	ax_trait_require(trie, trie->tr->exist);
	return trie->tr->exist(trie, key);
}

#endif
