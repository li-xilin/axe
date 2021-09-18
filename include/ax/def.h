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

#define AX_MIN(a, b) ((a) < (b) ? (a) : (b))
#define AX_MAX(a, b) ((a) > (b) ? (a) : (b))

#define AX_IMAX_BITS(m) ((m) /((m)%0x3fffffffL+1) /0x3fffffffL %0x3fffffffL *30 \
                  + (m)%0x3fffffffL /((m)%31+1)/31%31*5 + 4-12/((m)%31+3))

#define ax_block for(int __ax_block_flag = 0; __ax_block_flag != 1; __ax_block_flag = 1)

#define ax_unused(_var) ((void)&(_var))

#define ax_repeat(_n) for(size_t __ax_repeat_count = 0; __ax_repeat_count != (_n); __ax_repeat_count++)

#define ax_forever while(true)

#define ax_align(_size, _align) (!((_size) % (_align)) ? (_size) : ((_size) + (_align) - ((_size) % (_align))))

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

#define __AX_STRINGY(_x) #_x
#define AX_STRINGY(_x) __AX_STRINGY(_x)

/* Architecture Macros */

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
#	define AX_ARCH_AMD64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(__IA32__) || defined(__I86__) || \
	defined(_M_I386) || defined(_M_IX86) || defined(__INTEL) || defined(__THW_INTEL__)
#	define AX_ARCH_I386
#elif defined(__ia64__) || defined(__ia64) || defined(_M_IA64) || defined(__itanium__)
#	define AX_ARCH_IA64
#elif defined(__arm__) || defined(__arm) || defined(_M_ARM) || defined(_ARM)
#	define AX_ARCH_ARM
#	if defined(__ARM_ARCH_2__)
#		define AX_ARCH_ARM_2
#	elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#		define AX_ARCH_ARM_3
#	elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
#		define AX_ARCH_ARM_4T
#	elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__)
#		define AX_ARCH_ARM_5
#	elif defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
#		define AX_ARCH_ARM_5T
#	elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2__)
#		define AX_ARCH_ARM_6T2
#	elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
#		define AX_ARCH_ARM_6
#	elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#		define AX_ARCH_ARM_7
#	elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#		define AX_ARCH_ARM_7R
#	elif defined(__ARM_ARCH_7M__)
#		define AX_ARCH_ARM_7M
#	elif defined(__ARM_ARCH_7S__)
#		define AX_ARCH_ARM_7S
#	endif
#elif defined(__aarch64__) || defined(_M_ARM64)
#	define AX_ARCH_AARCH64
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__)
#	define AX_ARCH_MIPS
#elif defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#	define AX_ARCH_ALPHA
#	if defined(__alpha_ev4__)
#		define AX_ARCH_ALPHA_EV4
#	elif defined(__alpha_ev5__)
#		define AX_ARCH_ALPHA_EV5
#	elif defined(__alpha_ev6__)
#		define AX_ARCH_ALPHA_EV6
#	endif
#elif defined(__sh__)
#	define AX_ARCH_SUPERH
#elif defined(__PPC__) || defined(__ppc__) || defined(__ppc) || defined(__powerpc__) || defined(__POWERPC__) || defined(_M_PPC) || defined(_ARCH_PPC)
#	define AX_ARCH_PPC
#elif defined(__PPC64__) || defined(__ppc64__) || defined(__ppc64) || defined(__powerpc64__) || defined(_ARCH_PPC64)
#	define AX_ARCH_PPC64
#elif defined(__sparc__) || defined(__sparc)
#	define AX_ARCH_SPARC
#elif defined(__m68k__) || defined(__MC68K__) || defined(M68000)
#	define AX_ARCH_M64K
#elif defined(__convex__)
#	define AX_ARCH_CONVEX
#	if defined(__convex_c1__)
#		define AX_ARCH_CONVEX_C1
#	elif defined(__convex_c2__)
#		define AX_ARCH_CONVEX_C2
#	elif defined(__convex_c32__)
#		define AX_ARCH_CONVEX_C32
#	elif defined(__convex_c34__)
#		define AX_ARCH_CONVEX_C34
#	elif defined(__convex_c38__)
#		define AX_ARCH_CONVEX_C38
#	endif
#elif defined(__hppa__) || defined(__HPPA__) || defined(__hppa)
#	define AX_ARCH_HPPA
#	if defined(_PA_RISC1_0)
#		define AX_ARCH_HPPA_10
#	elif defined(_PA_RISC1_1) || defined(__PA7100__) || defined(__HPPA11__)
#		define AX_ARCH_HPPA_10
#	elif defined(_PA_RISC2_0) || defined(__RISC2_0__) || defined(__HPPA20__) || defined(__PA8000__)
#		define AX_ARCH_HPPA_10
#	endif
#endif

#endif
