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
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

typedef bool ax_fail;

typedef uint8_t ax_byte;

typedef uint_fast32_t ax_fast_uint;

typedef void (*ax_unary_f)(void *out, const void *in, void *arg);

typedef void (*ax_binary_f)(void *out, const void *in1, const void *in2, void *arg);

#define AX_MIN(a, b) ((a) < (b) ? a : b)
#define AX_MAX(a, b) ((a) > (b) ? a : b)

#define AX_IMAX_BITS(m) ((m) /((m)%0x3fffffffL+1) /0x3fffffffL %0x3fffffffL *30 \
                  + (m)%0x3fffffffL /((m)%31+1)/31%31*5 + 4-12/((m)%31+3))

#define ax_block for(int __ax_block_flag = 0; __ax_block_flag != 1; __ax_block_flag = 1)

#define ax_unused(_var) ((void)&(_var))

#define ax_repeat(_n) for(size_t __ax_repeat_count = 0; __ax_repeat_count != (_n); __ax_repeat_count++)

#define ax_forever while(true)

#define ax_align(_size, _align) (!((_size) % (_align)) ? (_size) : ((_size) + (_align) - ((_size) % (_align))))

#endif
