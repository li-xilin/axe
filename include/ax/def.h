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

/* Architecture Macros */

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
#	define AX_ARCH_AMD64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(__IA32__) || defined(__I86__) \
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
