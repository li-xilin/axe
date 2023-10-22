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

#ifndef AX_DEF_H
#define AX_DEF_H

#include "arraya.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef bool ax_fail;

typedef uint8_t ax_byte;

typedef uint_fast32_t ax_fast_uint;

#define AX_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AX_MAX(a, b) ((a) > (b) ? (a) : (b))
#define AX_INC(i, n) ((i + 1) % n)
#define AX_DEC(i, n) ((i + n - 1) % n)

#define AX_IMAX_BITS(m) ((m) /((m)%0x3fffffffL+1) /0x3fffffffL %0x3fffffffL *30 \
                  + (m)%0x3fffffffL /((m)%31+1)/31%31*5 + 4-12/((m)%31+3))

#define ax_unused(_var) ((void)&(_var))

#define ax_cast(_t, _v) (*(_t [1]) { _v })

#define ax_align(_size, _align) \
	((ax_cast(size_t, _size) + _align - 1) / ax_cast(size_t, _align) * (_align))

#define __ax_stringy(_x) #_x
#define ax_stringy(_x) __ax_stringy(_x)

#define ax_p(_type, ...) ((_type []) { __VA_ARGS__ })
#define ax_pstruct(_type, ...) ax_p(_type, { __VA_ARGS__ })

#define ax_strcommalen(s) ("" s), (sizeof(s) - 1)

#define ax_nelems(_a) (ax_assert((void *)_a == (void *)&_a, "_a pass to ax_nelems is not array"), \
		(sizeof(_a) / sizeof(_a[0])))

#define ax_container_of(ptr, type, member) (((type*)((char*)ptr - (offsetof(type, member)))))

#define ax_spair(_name, _value) ._name = (_value)

#endif

