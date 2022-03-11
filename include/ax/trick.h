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

#ifndef AX_TRICK_H
#define AX_TRICK_H

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
#define AX_MACRO_INC(n) __AX_PREFIX_TO_1(__AX_INC_##n)
#define AX_MACRO_DEC(n) __AX_PREFIX_TO_1(__AX_DEC_##n)

#define __AX_OVERLOAD_N(sym, n, ...) sym##n(__VA_ARGS__)
#define AX_OVERLOAD_N(sym, n, ...) __AX_OVERLOAD_N(sym, n, __VA_ARGS__)
#define AX_OVERLOAD(sym, ...) AX_OVERLOAD_N(sym, AX_NARG(__VA_ARGS__), __VA_ARGS__)

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

#define __AX_PREFIX_TO_0(_pre, x) x
#define __AX_PREFIX_TO_1(_pre, x) __AX_CALL_1(__AX_PREFIX_TO_0, _pre, _pre##x)
#define __AX_PREFIX_TO_2(_pre, x) __AX_CALL_2(__AX_PREFIX_TO_1, _pre, _pre##x)
#define __AX_PREFIX_TO_3(_pre, x) __AX_CALL_3(__AX_PREFIX_TO_2, _pre, _pre##x)
#define __AX_PREFIX_TO_4(_pre, x) __AX_CALL_4(__AX_PREFIX_TO_3, _pre, _pre##x)
#define __AX_PREFIX_TO_5(_pre, x) __AX_CALL_5(__AX_PREFIX_TO_4, _pre, _pre##x)
#define __AX_PREFIX_TO_6(_pre, x) __AX_CALL_6(__AX_PREFIX_TO_5, _pre, _pre##x)
#define __AX_PREFIX_TO_7(_pre, x) __AX_CALL_7(__AX_PREFIX_TO_6, _pre, _pre##x)
#define __AX_PREFIX_TO_8(_pre, x) __AX_CALL_8(__AX_PREFIX_TO_7, _pre, _pre##x)
#define __AX_PREFIX_TO_9(_pre, x) __AX_CALL_9(__AX_PREFIX_TO_8, _pre, _pre##x)
#define __AX_PREFIX_TO_10(_pre, x) __AX_CALL_10(__AX_PREFIX_TO_9, _pre, _pre##x)
#define __AX_PREFIX_TO_11(_pre, x) __AX_CALL_11(__AX_PREFIX_TO_10, _pre, _pre##x)
#define __AX_PREFIX_TO_12(_pre, x) __AX_CALL_12(__AX_PREFIX_TO_11, _pre, _pre##x)
#define __AX_PREFIX_TO_13(_pre, x) __AX_CALL_13(__AX_PREFIX_TO_12, _pre, _pre##x)
#define __AX_PREFIX_TO_14(_pre, x) __AX_CALL_14(__AX_PREFIX_TO_13, _pre, _pre##x)
#define __AX_PREFIX_TO_15(_pre, x) __AX_CALL_15(__AX_PREFIX_TO_14, _pre, _pre##x)
#define __AX_PREFIX_TO_16(_pre, x) __AX_CALL_16(__AX_PREFIX_TO_15, _pre, _pre##x)
#define AX_PREFIX_TO(n, _pre, x) __AX_CALL_##n(__AX_PREFIX_TO_##n, _pre, x)

#define __AX_PAVE_TO_0(n, macro, ...)
#define __AX_PAVE_TO_1(n, macro, ...) macro(n, __VA_ARGS__)
#define __AX_PAVE_TO_2(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_1(1, macro, __VA_ARGS__)
#define __AX_PAVE_TO_3(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_2(2, macro, __VA_ARGS__)
#define __AX_PAVE_TO_4(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_3(3, macro, __VA_ARGS__)
#define __AX_PAVE_TO_5(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_4(4, macro, __VA_ARGS__)
#define __AX_PAVE_TO_6(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_5(5, macro, __VA_ARGS__)
#define __AX_PAVE_TO_7(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_6(6, macro, __VA_ARGS__)
#define __AX_PAVE_TO_8(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_7(7, macro, __VA_ARGS__)
#define __AX_PAVE_TO_9(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_8(8, macro, __VA_ARGS__)
#define __AX_PAVE_TO_10(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_9(9, macro, __VA_ARGS__)
#define __AX_PAVE_TO_11(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_10(10, macro, __VA_ARGS__)
#define __AX_PAVE_TO_12(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_11(11, macro, __VA_ARGS__)
#define __AX_PAVE_TO_13(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_12(12, macro, __VA_ARGS__)
#define __AX_PAVE_TO_14(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_13(13, macro, __VA_ARGS__)
#define __AX_PAVE_TO_15(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_14(14, macro, __VA_ARGS__)
#define __AX_PAVE_TO_16(n, macro, ...) macro(n, __VA_ARGS__) __AX_PAVE_TO_15(15, macro, __VA_ARGS__)
#define AX_PAVE_TO(n, macro, ...) __AX_PAVE_TO_##n(n, macro, __VA_ARGS__)

#define __AX_CATENATE_8(a1, a2, a3, a4, a5, a6, a7, a8) a1##a2##a3##a4##a5##a6##a7##a8
#define __AX_CATENATE_7(a1, a2, a3, a4, a5, a6, a7) a1##a2##a3##a4##a5##a6##a7
#define __AX_CATENATE_6(a1, a2, a3, a4, a5, a6) a1##a2##a3##a4##a5##a6
#define __AX_CATENATE_5(a1, a2, a3, a4, a5) a1##a2##a3##a4##a5
#define __AX_CATENATE_4(a1, a2, a3, a4) a1##a2##a3##a4
#define __AX_CATENATE_3(a1, a2, a3) a1##a2##a3
#define __AX_CATENATE_2(a1, a2) a1##a2
#define __AX_CATENATE_1(a1) a1
#define AX_CATENATE(...) AX_OVERLOAD(__AX_CATENATE_, __VA_ARGS__)

#endif
