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

#ifndef AX_ANY_H
#define AX_ANY_H
#include "one.h"

#define AX_ANY_NAME AX_ONE_NAME ".any"

#ifndef AX_STUFF_TRAIT_DEFINED
#define AX_STUFF_TRAIT_DEFINED
typedef struct ax_stuff_trait_st ax_stuff_trait;
#endif

#ifndef AX_DUMP_DEFINED
#define AX_DUMP_DEFINED
typedef struct ax_dump_st ax_dump;
#endif

#ifndef AX_ANY_DEFINED
#define AX_ANY_DEFINED
typedef struct ax_any_st ax_any;
#endif

#ifndef AX_ANY_TRAIT_DEFINED
#define AX_ANY_TRAIT_DEFINED
typedef struct ax_any_trait_st ax_any_trait;
#endif

typedef ax_any *(*ax_any_copy_f)(const ax_any* any);
typedef ax_dump *(*ax_any_dump_f)(const ax_any* any);
//typedef void (*ax_any_dump_f)(const ax_any *any, int ind);

struct ax_any_trait_st
{
	const ax_one_trait one;
	const ax_any_dump_f dump;
	const ax_any_copy_f copy;
};

typedef union
{
	const ax_any *any;
	const ax_one *one;
} ax_any_cr;

typedef union
{
	ax_any *any;
	ax_one *one;
	ax_any_cr c;
} ax_any_r;

typedef struct ax_any_env_st
{
	ax_one_env one;
} ax_any_env;

struct ax_any_st
{
	const ax_any_trait *const tr;
	ax_any_env env;
};

inline static ax_any *ax_any_copy(const ax_any* any)
{
	ax_trait_require(any, any->tr->copy);
	return any->tr->copy(any);
}

inline static ax_dump *ax_any_dump(const ax_any* any)
{
	ax_trait_require(any, any->tr->dump);
	return any->tr->dump(any);
}

ax_fail ax_any_so(const ax_any *any);

extern const ax_stuff_trait ax_any_tr;

ax_any *ax_any_seal(ax_scope *scope, ax_any *any);

#endif

