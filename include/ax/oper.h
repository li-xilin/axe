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

#ifndef AX_OPER_H
#define AX_OPER_H

typedef void (*ax_unary_f)(void *out, const void *in, void *arg);

typedef void (*ax_binary_f)(void *out, const void *in1, const void *in2, void *arg);

typedef struct ax_oper_st ax_oper;

struct ax_oper_st
{
    ax_binary_f o_add;
    ax_binary_f o_sub;
    ax_binary_f o_mul;
    ax_binary_f o_div;
    ax_binary_f o_mod;

    ax_binary_f o_and;
    ax_binary_f o_or;
    ax_unary_f  o_not;

    ax_binary_f o_bit_and;
    ax_binary_f o_bit_or;
    ax_unary_f  o_bit_not;
    ax_binary_f o_bit_xor;

    ax_binary_f o_gt;
    ax_binary_f o_ge;
    ax_binary_f o_lt;
    ax_binary_f o_le;
    ax_binary_f o_eq;
    ax_binary_f o_ne;

    ax_unary_f o_hash;
};

extern const ax_oper ax_oper_int8_t;
extern const ax_oper ax_oper_int16_t;
extern const ax_oper ax_oper_int32_t;
extern const ax_oper ax_oper_int64_t;
extern const ax_oper ax_oper_uint8_t;
extern const ax_oper ax_oper_uint16_t;
extern const ax_oper ax_oper_uint32_t;
extern const ax_oper ax_oper_uint64_t;
extern const ax_oper ax_oper_uintptr_t;
extern const ax_oper ax_oper_size_t;
extern const ax_oper ax_oper_float;
extern const ax_oper ax_oper_double;

#define ax_op(type) ax_oper_##type

#endif
