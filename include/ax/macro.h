/*
 * Copyright (c) 2022 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_MACRO_H
#define AX_MACRO_H

#define __AX_INC_0 1
#define __AX_INC_1 2
#define __AX_INC_2 3
#define __AX_INC_3 4
#define __AX_INC_4 5
#define __AX_INC_5 6
#define __AX_INC_6 7
#define __AX_INC_7 8
#define __AX_INC_8 9
#define __AX_INC_9 10
#define __AX_INC_10 11
#define __AX_INC_11 12
#define __AX_INC_12 13
#define __AX_INC_13 14
#define __AX_INC_14 15
#define __AX_INC_15 16

#define __AX_DEC_1 0
#define __AX_DEC_2 1
#define __AX_DEC_3 2
#define __AX_DEC_4 3
#define __AX_DEC_5 4
#define __AX_DEC_6 5
#define __AX_DEC_7 6
#define __AX_DEC_8 7
#define __AX_DEC_9 8
#define __AX_DEC_10 9
#define __AX_DEC_11 10
#define __AX_DEC_12 11
#define __AX_DEC_13 12
#define __AX_DEC_14 13
#define __AX_DEC_15 14
#define __AX_DEC_16 15

#define __AX_CALL_1(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_2(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_3(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_4(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_5(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_6(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_7(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_8(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_9(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_10(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_11(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_12(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_13(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_14(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_15(macro, ...) macro(__VA_ARGS__)
#define __AX_CALL_16(macro, ...) macro(__VA_ARGS__)

#define __AX_MACRO_EXPAND_0(_pre, x) x
#define __AX_MACRO_EXPAND_1(_pre, x) __AX_CALL_1(__AX_MACRO_EXPAND_0, _pre, _pre##x)
#define __AX_MACRO_EXPAND_2(_pre, x) __AX_CALL_2(__AX_MACRO_EXPAND_1, _pre, _pre##x)
#define __AX_MACRO_EXPAND_3(_pre, x) __AX_CALL_3(__AX_MACRO_EXPAND_2, _pre, _pre##x)
#define __AX_MACRO_EXPAND_4(_pre, x) __AX_CALL_4(__AX_MACRO_EXPAND_3, _pre, _pre##x)
#define __AX_MACRO_EXPAND_5(_pre, x) __AX_CALL_5(__AX_MACRO_EXPAND_4, _pre, _pre##x)
#define __AX_MACRO_EXPAND_6(_pre, x) __AX_CALL_6(__AX_MACRO_EXPAND_5, _pre, _pre##x)
#define __AX_MACRO_EXPAND_7(_pre, x) __AX_CALL_7(__AX_MACRO_EXPAND_6, _pre, _pre##x)
#define __AX_MACRO_EXPAND_8(_pre, x) __AX_CALL_8(__AX_MACRO_EXPAND_7, _pre, _pre##x)
#define __AX_MACRO_EXPAND_9(_pre, x) __AX_CALL_9(__AX_MACRO_EXPAND_8, _pre, _pre##x)
#define __AX_MACRO_EXPAND_10(_pre, x) __AX_CALL_10(__AX_MACRO_EXPAND_9, _pre, _pre##x)
#define __AX_MACRO_EXPAND_11(_pre, x) __AX_CALL_11(__AX_MACRO_EXPAND_10, _pre, _pre##x)
#define __AX_MACRO_EXPAND_12(_pre, x) __AX_CALL_12(__AX_MACRO_EXPAND_11, _pre, _pre##x)
#define __AX_MACRO_EXPAND_13(_pre, x) __AX_CALL_13(__AX_MACRO_EXPAND_12, _pre, _pre##x)
#define __AX_MACRO_EXPAND_14(_pre, x) __AX_CALL_14(__AX_MACRO_EXPAND_13, _pre, _pre##x)
#define __AX_MACRO_EXPAND_15(_pre, x) __AX_CALL_15(__AX_MACRO_EXPAND_14, _pre, _pre##x)
#define __AX_MACRO_EXPAND_16(_pre, x) __AX_CALL_16(__AX_MACRO_EXPAND_15, _pre, _pre##x)

#define __AX_MACRO_PAVE_0(macro, n, ...)
#define __AX_MACRO_PAVE_1(macro, n, ...) macro(n, __VA_ARGS__)
#define __AX_MACRO_PAVE_2(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_1(macro, 1, __VA_ARGS__)
#define __AX_MACRO_PAVE_3(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_2(macro, 2, __VA_ARGS__)
#define __AX_MACRO_PAVE_4(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_3(macro, 3, __VA_ARGS__)
#define __AX_MACRO_PAVE_5(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_4(macro, 4, __VA_ARGS__)
#define __AX_MACRO_PAVE_6(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_5(macro, 5, __VA_ARGS__)
#define __AX_MACRO_PAVE_7(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_6(macro, 6, __VA_ARGS__)
#define __AX_MACRO_PAVE_8(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_7(macro, 7, __VA_ARGS__)
#define __AX_MACRO_PAVE_9(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_8(macro, 8, __VA_ARGS__)
#define __AX_MACRO_PAVE_10(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_9(macro, 9, __VA_ARGS__)
#define __AX_MACRO_PAVE_11(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_10(macro, 10, __VA_ARGS__)
#define __AX_MACRO_PAVE_12(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_11(macro, 11, __VA_ARGS__)
#define __AX_MACRO_PAVE_13(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_12(macro, 12, __VA_ARGS__)
#define __AX_MACRO_PAVE_14(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_13(macro, 13, __VA_ARGS__)
#define __AX_MACRO_PAVE_15(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_14(macro, 14, __VA_ARGS__)
#define __AX_MACRO_PAVE_16(macro, n, ...) macro(n, __VA_ARGS__) __AX_MACRO_PAVE_15(macro, 15, __VA_ARGS__)

#define AX_MACRO_PAVE(n, macro, ...) __AX_MACRO_PAVE_##n(macro, n, __VA_ARGS__)
#define AX_MACRO_INC(n) __AX_MACRO_EXPAND_1(__AX_INC_##n)
#define AX_MACRO_DEC(n) __AX_MACRO_EXPAND_1(__AX_DEC_##n)

#define AX_MACRO_EXPAND(n, _pre, x) __AX_CALL_##n(__AX_MACRO_EXPAND_15, _pre, x)

#endif
