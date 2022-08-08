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

#include "ax/oper.h"
#include "ax/trait.h"
#include "ax/def.h"

#define ptr_add(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 + *(_type*)_in2)
#define ptr_sub(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 - *(_type*)_in2)
#define ptr_mul(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 * *(_type*)_in2)
#define ptr_div(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 / *(_type*)_in2)
#define ptr_mod(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 % *(_type*)_in2)

#define ptr_and(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 && *(_type*)_in2)
#define ptr_or(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 || *(_type*)_in2)
#define ptr_not(_type, _in, _out) (*(bool*)_out = !*(_type*)_in)

#define ptr_bit_and(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 & *(_type*)_in2)
#define ptr_bit_or(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 | *(_type*)_in2)
#define ptr_bit_xor(_type, _in1, _in2, _out) (*(_type*)_out = *(_type*)_in1 ^ *(_type*)_in2)
#define ptr_bit_not(_type, _in, _out) (*(_type*)_out = ~*(_type*)_in)

#define ptr_gt(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 > *(_type*)_in2)
#define ptr_ge(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 >= *(_type*)_in2)
#define ptr_lt(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 < *(_type*)_in2)
#define ptr_le(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 <= *(_type*)_in2)
#define ptr_eq(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 == *(_type*)_in2)
#define ptr_ne(_type, _in1, _in2, _out) (*(bool*)_out = *(_type*)_in1 != *(_type*)_in2)

#define DECLARE_UNARY_OPER(_type, _op) \
	static void _type##_op(void *out, const void *in, void *arg) { \
		ptr##_op(_type, in, out); \
	}

#define DECLARE_BINARY_OPER(_type, _op) \
	static void _type##_op(void *out, const void *in1, const void *in2, void *arg) { \
		ptr##_op(_type, in1, in2, out); \
	}


#define DECLARE_INT_OPERSET_FUNCS(_type) \
	DECLARE_BINARY_OPER(_type, _add) \
	DECLARE_BINARY_OPER(_type, _sub) \
	DECLARE_BINARY_OPER(_type, _mul) \
	DECLARE_BINARY_OPER(_type, _div) \
	DECLARE_BINARY_OPER(_type, _mod) \
	DECLARE_BINARY_OPER(_type, _and) \
	DECLARE_BINARY_OPER(_type, _or) \
	DECLARE_UNARY_OPER(_type, _not) \
	DECLARE_BINARY_OPER(_type, _bit_and) \
	DECLARE_BINARY_OPER(_type, _bit_or) \
	DECLARE_BINARY_OPER(_type, _bit_xor) \
	DECLARE_UNARY_OPER(_type, _bit_not) \
	DECLARE_BINARY_OPER(_type, _gt) \
	DECLARE_BINARY_OPER(_type, _ge) \
	DECLARE_BINARY_OPER(_type, _lt) \
	DECLARE_BINARY_OPER(_type, _le) \
	DECLARE_BINARY_OPER(_type, _eq) \
	DECLARE_BINARY_OPER(_type, _ne)


#define DECLARE_INT_OPERSET_STRUCT(_type) \
	const struct ax_oper_st ax_oper_##_type = \
	{ \
		.add = _type##_add, \
		.sub = _type##_sub, \
		.mul = _type##_mul, \
		.div = _type##_div, \
		.mod = _type##_mod, \
		.and = _type##_and, \
		.or  = _type##_or, \
		.not = _type##_not, \
		.bit_and = _type##_bit_and, \
		.bit_or  = _type##_bit_or, \
		.bit_not = _type##_bit_not, \
		.bit_xor = _type##_bit_xor, \
		.gt  = _type##_gt, \
		.ge  = _type##_ge, \
		.lt  = _type##_lt, \
		.le  = _type##_le, \
		.eq  = _type##_eq, \
		.ne  = _type##_ne, \
		.hash = NULL \
	};

#define DECLARE_INT_OPER(_type) \
	DECLARE_INT_OPERSET_FUNCS(_type) \
	DECLARE_INT_OPERSET_STRUCT(_type)

DECLARE_INT_OPER(int8_t)
DECLARE_INT_OPER(int16_t)
DECLARE_INT_OPER(int32_t)
DECLARE_INT_OPER(int64_t)
DECLARE_INT_OPER(uint8_t)
DECLARE_INT_OPER(uint16_t)
DECLARE_INT_OPER(uint32_t)
DECLARE_INT_OPER(uint64_t)
DECLARE_INT_OPER(size_t)

#define DECLARE_FLOAT_OPERSET_FUNCS(_type) \
	DECLARE_BINARY_OPER(_type, _add) \
	DECLARE_BINARY_OPER(_type, _sub) \
	DECLARE_BINARY_OPER(_type, _mul) \
	DECLARE_BINARY_OPER(_type, _div) \
	DECLARE_BINARY_OPER(_type, _gt) \
	DECLARE_BINARY_OPER(_type, _ge) \
	DECLARE_BINARY_OPER(_type, _lt) \
	DECLARE_BINARY_OPER(_type, _le) \
	DECLARE_BINARY_OPER(_type, _eq) \
	DECLARE_BINARY_OPER(_type, _ne)

#define DECLARE_FLOAT_OPERSET_STRUCT(_type) \
	const struct ax_oper_st ax_oper_##_type = \
	{ \
		.add = _type##_add, \
		.sub = _type##_sub, \
		.mul = _type##_mul, \
		.div = _type##_div, \
		.mod = NULL, \
		\
		.and = NULL, \
		.or  = NULL, \
		.not = NULL, \
		\
		.bit_and = NULL, \
		.bit_or  = NULL, \
		.bit_not = NULL, \
		.bit_xor = NULL, \
		\
		.gt  = _type##_gt, \
		.ge  = _type##_ge, \
		.lt  = _type##_lt, \
		.le  = _type##_le, \
		.eq  = _type##_eq, \
		.ne  = _type##_ne, \
		\
		.hash = NULL \
	};


DECLARE_FLOAT_OPERSET_FUNCS(float)
DECLARE_FLOAT_OPERSET_FUNCS(double)
DECLARE_FLOAT_OPERSET_STRUCT(float)
DECLARE_FLOAT_OPERSET_STRUCT(double)
