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

#include "narg.h"
#include "arraya.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef bool ax_fail;

typedef uint8_t ax_byte;

typedef uint_fast32_t ax_fast_uint;

#define AX_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AX_MAX(a, b) ((a) > (b) ? (a) : (b))

#define AX_IMAX_BITS(m) ((m) /((m)%0x3fffffffL+1) /0x3fffffffL %0x3fffffffL *30 \
                  + (m)%0x3fffffffL /((m)%31+1)/31%31*5 + 4-12/((m)%31+3))

#define ax_unused(_var) ((void)&(_var))

#define ax_block for(int __ax_block_flag = 0; __ax_block_flag != 1; __ax_block_flag = 1)

#define ax_repeat(_n) for(size_t _ = 0; _ != (_n); _++)

#define ax_forever for(;;)

#define ax_align(_size, _align) \
	(!((_size) % (_align)) ? (_size) \
	 : ((_size) + (_align) - ((_size) % (_align))))

#define __AX_CATENATE_8(a1, a2, a3, a4, a5, a6, a7, a8) a1##a2##a3##a4##a5##a6##a7##a8
#define __AX_CATENATE_7(a1, a2, a3, a4, a5, a6, a7) a1##a2##a3##a4##a5##a6##a7
#define __AX_CATENATE_6(a1, a2, a3, a4, a5, a6) a1##a2##a3##a4##a5##a6
#define __AX_CATENATE_5(a1, a2, a3, a4, a5) a1##a2##a3##a4##a5
#define __AX_CATENATE_4(a1, a2, a3, a4) a1##a2##a3##a4
#define __AX_CATENATE_3(a1, a2, a3) a1##a2##a3
#define __AX_CATENATE_2(a1, a2) a1##a2
#define __AX_CATENATE_1(a1) a1

#define __AX_CATENATE1_N(n, ...) __AX_CATENATE_##n(__VA_ARGS__)
#define __AX_CATENATE_N(n, ...) __AX_CATENATE1_N(n, __VA_ARGS__)
#define AX_CATENATE(...) __AX_CATENATE_N(AX_NARG(__VA_ARGS__), __VA_ARGS__)

#define __ax_stringy(_x) #_x
#define ax_stringy(_x) __ax_stringy(_x)

#define ax_ptrof(_t, _v) ((_t [1]) { _v })
#define ax_p(_type, _value) ((_type [1]) { _value })

#define ax_strcommalen(s) ("" s), (sizeof(s) - 1)
#define ax_vstrcommalen(s) (s), strlen(s)

#define ax_nelems(_a) (ax_assert((void *)_a == (void *)&_a, "_a pass to ax_nelems is not array"), \
		(sizeof(_a) / sizeof(*_a)))

#endif

