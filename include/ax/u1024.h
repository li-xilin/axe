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

#ifndef AX_U1024_H
#define AX_U1024_H

#include "def.h"
#include <stdint.h>

#define AX_U1024_ARR_LEN 32
#define AX_U1024_MAX ((uint64_t)0xFFFFFFFF)
#define AX_U1024_WORD_SIZE (sizeof(uint32_t))

struct ax_u1024_st
{
	uint32_t array[AX_U1024_ARR_LEN];
};

#ifndef AX_U1024_DEFINED
#define AX_U1024_DEFINED
typedef struct ax_u1024_st ax_u1024;
#endif

enum {
	AX_U1024_SMALLER = -1,
	AX_U1024_EQUAL = 0,
	AX_U1024_LARGER = 1
};

/* Initialization functions: */
void ax_u1024_init(ax_u1024* n);
void ax_u1024_from_int(ax_u1024* n, uint64_t i);
int  ax_u1024_to_int(ax_u1024* n);
void ax_u1024_from_string(ax_u1024* n, char* str, int nbytes);
void ax_u1024_to_string(ax_u1024* n, char* str, int maxsize);

/* Basic arithmetic operations: */
void ax_u1024_add(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* c = a + b */
void ax_u1024_sub(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* c = a - b */
void ax_u1024_mul(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* c = a * b */
void ax_u1024_div(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* c = a / b */
void ax_u1024_mod(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* c = a % b */
void ax_u1024_divmod(const ax_u1024* a, const ax_u1024* b, ax_u1024* c, ax_u1024* d); /* c = a/b, d = a%b */

/* Bitwise operations: */
void ax_u1024_and(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* c = a & b */
void ax_u1024_or(const ax_u1024* a, const ax_u1024* b, ax_u1024* c);  /* c = a | b */
void ax_u1024_xor(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* c = a ^ b */
void ax_u1024_lshift(const ax_u1024* a, ax_u1024* b, int nbits);      /* b = a << nbits */
void ax_u1024_rshift(const ax_u1024* a, ax_u1024* b, int nbits);      /* b = a >> nbits */

/* Special operators and comparison */
int  ax_u1024_cmp(const ax_u1024* a, const ax_u1024* b);              /* Compare: returns AX_U1024_LARGER, AX_U1024_EQUAL or AX_U1024_SMALLER */
int  ax_u1024_is_zero(const ax_u1024* n);                                   /* For comparison with zero */
void ax_u1024_inc(ax_u1024* n);                                       /* Increment: add one to n */
void ax_u1024_dec(ax_u1024* n);                                       /* Decrement: subtract one from n */
void ax_u1024_pow(const ax_u1024* a, const ax_u1024* b, ax_u1024* c); /* Calculate a^b -- e.g. 2^10 => 1024 */
void ax_u1024_isqrt(const ax_u1024* a, ax_u1024* b);                  /* Integer square root -- e.g. isqrt(5) => 2*/
void ax_u1024_assign(ax_u1024* dst, const ax_u1024* src);             /* Copy src into dst -- dst := src */

#endif
