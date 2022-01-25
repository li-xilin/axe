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

#ifndef AX_NARG_H
#define AX_NARG_H

#define AX_NARG(...) \
         __AX_NARG_(__VA_ARGS__, __AX_RSEQ_N)
#define __AX_NARG_(...) \
         __AX_ARG_N(__VA_ARGS__)
#define __AX_ARG_N( \
          _1, _2, _3, _4, _5, _6, _7, _8, _9,_10, \
         _11,_12,_13,_14,_15,_16,_17,_18,_19,_20, \
         _21,_22,_23,_24,_25,_26,_27,_28,_29,_30, \
         _31,_32,_33,_34,_35,_36,_37,_38,_39,_40, \
         _41,_42,_43,_44,_45,_46,_47,_48,_49,_50, \
         _51,_52,_53,_54,_55,_56,_57,_58,_59,_60, \
         _61,_62,_63,N,...) N
#define __AX_RSEQ_N \
         63, 62, 61, 60,                          \
         59, 58, 57, 56, 55, 54, 53, 52, 51, 50,  \
         49, 48, 47, 46, 45, 44, 43, 42, 41, 40,  \
         39, 38, 37, 36, 35, 34, 33, 32, 31, 30,  \
         29, 28, 27, 26, 25, 24, 23, 22, 21, 20,  \
         19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  \
          9,  8,  7,  6,  5,  4,  3,  2,  1,  0

#define AX_NARG_T(_t, ...) (sizeof((_t[]){ __VA_ARGS__ }) / sizeof(_t))

#endif
