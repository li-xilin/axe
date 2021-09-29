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

#define AX_BTRIE_NAME AX_TRIE_NAME ".btrie"

#ifndef AX_BTRIE_DEFINED
#define AX_BTRIE_DEFINED
typedef struct ax_btrie_st ax_btrie;
#endif

#define AX_CLASS_BASE_btrie trie
#define AX_CLASS_ROLE_btrie(_l) _l AX_CLASS_PTR(btrie); AX_CLASS_ROLE_trie(_l)

AX_CLASS_STRUCT_ROLE(btrie);

extern const ax_trie_trait ax_btrie_tr;

ax_trie *__ax_btrie_construct(const ax_stuff_trait* key_tr, const ax_stuff_trait* val_tr);

inline static AX_CLASS_CONSTRUCTOR(btrie, const ax_stuff_trait* key_tr, const ax_stuff_trait* val_tr)
{
	return __ax_btrie_construct(key_tr, val_tr);
}

void ax_btrie_dump(ax_btrie *btrie);

#endif
