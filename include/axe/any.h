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

#ifndef AXE_ANY_H_
#define AXE_ANY_H_
#include "one.h"

#define AX_ANY_NAME "one.any"

#ifndef AX_STUFF_TRAIT_DEFINED
#define AX_STUFF_TRAIT_DEFINED
typedef struct ax_stuff_trait_st ax_stuff_trait;
#endif

#ifndef AX_ANY_DEFINED
#define AX_ANY_DEFINED
typedef struct ax_any_st ax_any;
#endif


#ifndef AX_ANY_TRAIT_DEFINED
#define AX_ANY_TRAIT_DEFINED
typedef struct ax_any_trait_st ax_any_trait;
#endif


typedef ax_any     *(*ax_any_copy_f) (const ax_any* any);
typedef ax_any     *(*ax_any_move_f) (      ax_any* any);
typedef void        (*ax_any_dump_f) (const ax_any* any, int ind); /* to be determined */
typedef void        (*ax_any_assign_f) (const ax_any* any); /* to be determined */

struct ax_any_trait_st
{
	const ax_any_dump_f  dump;
	const ax_any_copy_f  copy;
	const ax_any_move_f  move;
};


typedef union
{
	const ax_any *any;
	const ax_one *one;
} ax_any_crol;

typedef union
{
	ax_any *any;
	ax_one *one;
	ax_any_crol c;
} ax_any_role;

struct ax_any_st
{
	ax_one __one;
	const ax_any_trait *const tr;
};

inline static ax_any *ax_any_copy(const ax_any* any)
{
	return any->tr->copy(any);
}

inline static ax_any *ax_any_move(ax_any* any)
{
	return any->tr->move(any);
}

const ax_stuff_trait *ax_any_stuff_trait();

#endif

