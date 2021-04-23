/*
  *Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
 *
  *Permission is hereby granted, free of charge, to any person obtaining a copy
  *of this software and associated documentation files (the "Software"), to deal
  *in the Software without restriction, including without limitation the rights
  *to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  *copies of the Software, and to permit persons to whom the Software is
  *furnished to do so, subject to the following conditions:
 *
  *The above copyright notice and this permission notice shall be included in
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

#ifndef AXE_TUBE_H_
#define AXE_TUBE_H_
#include "box.h"
#include "def.h"

#define AX_TUBE_NAME AX_BOX_NAME ".tube"

#ifndef AX_TUBE_DEFINED
#define AX_TUBE_DEFINED
typedef struct ax_tube_st ax_tube;
#endif

#ifndef AX_TUBE_TRAIT_DEFINED
#define AX_TUBE_TRAIT_DEFINED
typedef struct ax_tube_trait_st ax_tube_trait;
#endif

typedef ax_fail (*ax_tube_push_f)   (ax_tube *tube, const void *val);
typedef void    (*ax_tube_pop_f)    (ax_tube *tube);
typedef void   *(*ax_tube_prime_f)  (const ax_tube *tube);
typedef size_t  (*ax_tube_size_f)   (const ax_tube *tube);

typedef ax_tube *(ax_tube_construct_f)(ax_base *base, const ax_stuff_trait *elem_tr);

struct ax_tube_trait_st
{
	const ax_any_trait any; /* Keep this first */
	const ax_tube_push_f   push;
	const ax_tube_pop_f    pop;
	const ax_tube_prime_f  prime;
	const ax_tube_size_f   size;
};

typedef struct ax_tube_env_st
{
	ax_one_env one; /* Keep this first */
	const ax_stuff_trait *const elem_tr;
} ax_tube_env;

struct ax_tube_st
{
	const ax_tube_trait *const tr; /* Keep this first */
	ax_tube_env env;
};

typedef union
{
	const ax_tube *tube;
	const ax_any *any;
	const ax_one *one;
} ax_tube_cr;

typedef union
{
	ax_tube *tube;
	ax_any *any;
	ax_one *one;
	ax_tube_cr c;
} ax_tube_r;

inline static ax_fail ax_tube_push(ax_tube *tube, const void *val)
{
	ax_trait_require(tube, tube->tr->push);
	return tube->tr->push(tube, val);
}

inline static void ax_tube_pop(ax_tube *tube)
{
	ax_trait_require(tube, tube->tr->pop);
	tube->tr->pop(tube);
}

static inline void *ax_tube_prime(ax_tube *tube)
{
	ax_trait_optional(tube, tube->tr->last);
	return tube->tr->prime(tube);
}

static inline const void *ax_tube_cprime(const ax_tube *tube)
{
	ax_trait_optional(tube, tube->tr->last);
	return tube->tr->prime(tube);
}

static inline size_t ax_tube_size(const ax_tube *tube)
{
	ax_trait_optional(tube, tube->tr->last);
	return tube->tr->size(tube);
}

#endif
