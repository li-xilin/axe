/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_BTRIE_H
#define AX_BTRIE_H
#include "trie.h"

#define AX_BTRIE_NAME AX_SEQ_NAME ".btrie"

typedef struct ax_btrie_st ax_btrie;

typedef union
{
	const ax_btrie* btrie;
	const ax_trie* trie;
	const ax_box* box;
	const ax_any* any;
	const ax_one* one;
} ax_btrie_cr;

typedef union
{
	ax_btrie* btrie;
	ax_trie* trie;
	ax_seq* seq;
	ax_box* box;
	ax_any* any;
	ax_one* one;
	ax_btrie_cr c;
} ax_btrie_r;

extern const ax_trie_trait ax_btrie_tr;

ax_trie *__ax_btrie_construct(const ax_stuff_trait* key_tr, const ax_stuff_trait* val_tr);

ax_btrie_r ax_btrie_create(ax_scope* scope, const ax_stuff_trait* key_tr, const ax_stuff_trait* val_tr);

void ax_btrie_dump(ax_btrie *btrie);

#endif
