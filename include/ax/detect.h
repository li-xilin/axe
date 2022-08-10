/*
 * Copyright (c) 2021-2022 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_ARCH_H
#define AX_ARCH_H

/* detect architecture */

#if defined(__x86_64__) || defined(__x86_64) || defined(__amd64__) || defined(_M_X64) || defined(_M_AMD64)
#  define AX_ARCH_AMD64
#elif defined(i386) || defined(__i386__) || defined(__i386) || defined(__IA32__) || defined(__I86__) || \
  defined(_M_I386) || defined(_M_IX86) || defined(__INTEL) || defined(__THW_INTEL__)
#  define AX_ARCH_I386
#elif defined(__ia64__) || defined(__ia64) || defined(_M_IA64) || defined(__itanium__)
#  define AX_ARCH_IA64
#elif defined(__arm__) || defined(__arm) || defined(_M_ARM) || defined(_ARM)
#  define AX_ARCH_ARM
#  if defined(__ARM_ARCH_2__)
#    define AX_ARCH_ARM_2
#  elif defined(__ARM_ARCH_3__) || defined(__ARM_ARCH_3M__)
#    define AX_ARCH_ARM_3
#  elif defined(__ARM_ARCH_4T__) || defined(__TARGET_ARM_4T)
#    define AX_ARCH_ARM_4T
#  elif defined(__ARM_ARCH_5__) || defined(__ARM_ARCH_5E__)
#    define AX_ARCH_ARM_5
#  elif defined(__ARM_ARCH_5T__) || defined(__ARM_ARCH_5TE__) || defined(__ARM_ARCH_5TEJ__)
#    define AX_ARCH_ARM_5T
#  elif defined(__ARM_ARCH_6T2_) || defined(__ARM_ARCH_6T2__)
#    define AX_ARCH_ARM_6T2
#  elif defined(__ARM_ARCH_6__) || defined(__ARM_ARCH_6J__) || defined(__ARM_ARCH_6K__) || defined(__ARM_ARCH_6Z__) || defined(__ARM_ARCH_6ZK__)
#    define AX_ARCH_ARM_6
#  elif defined(__ARM_ARCH_7__) || defined(__ARM_ARCH_7A__) || defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#    define AX_ARCH_ARM_7
#  elif defined(__ARM_ARCH_7R__) || defined(__ARM_ARCH_7M__) || defined(__ARM_ARCH_7S__)
#    define AX_ARCH_ARM_7R
#  elif defined(__ARM_ARCH_7M__)
#    define AX_ARCH_ARM_7M
#  elif defined(__ARM_ARCH_7S__)
#    define AX_ARCH_ARM_7S
#  endif
#elif defined(__aarch64__) || defined(_M_ARM64)
#  define AX_ARCH_AARCH64
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__)
#  define AX_ARCH_MIPS
#elif defined(__alpha__) || defined(__alpha) || defined(_M_ALPHA)
#  define AX_ARCH_ALPHA
#  if defined(__alpha_ev4__)
#    define AX_ARCH_ALPHA_EV4
#  elif defined(__alpha_ev5__)
#    define AX_ARCH_ALPHA_EV5
#  elif defined(__alpha_ev6__)
#    define AX_ARCH_ALPHA_EV6
#  endif
#elif defined(__sh__)
#  define AX_ARCH_SUPERH
#elif defined(__PPC__) || defined(__ppc__) || defined(__ppc) || defined(__powerpc__) || defined(__POWERPC__) || defined(_M_PPC) || defined(_ARCH_PPC)
#  define AX_ARCH_PPC
#elif defined(__PPC64__) || defined(__ppc64__) || defined(__ppc64) || defined(__powerpc64__) || defined(_ARCH_PPC64)
#  define AX_ARCH_PPC64
#elif defined(__sparc__) || defined(__sparc)
#  define AX_ARCH_SPARC
#elif defined(__m68k__) || defined(__MC68K__) || defined(M68000)
#  define AX_ARCH_M64K
#elif defined(__convex__)
#  define AX_ARCH_CONVEX
#  if defined(__convex_c1__)
#    define AX_ARCH_CONVEX_C1
#  elif defined(__convex_c2__)
#    define AX_ARCH_CONVEX_C2
#  elif defined(__convex_c32__)
#    define AX_ARCH_CONVEX_C32
#  elif defined(__convex_c34__)
#    define AX_ARCH_CONVEX_C34
#  elif defined(__convex_c38__)
#    define AX_ARCH_CONVEX_C38
#  endif
#elif defined(__hppa__) || defined(__HPPA__) || defined(__hppa)
#  define AX_ARCH_HPPA
#  if defined(_PA_RISC1_0)
#    define AX_ARCH_HPPA_10
#  elif defined(_PA_RISC1_1) || defined(__PA7100__) || defined(__HPPA11__)
#    define AX_ARCH_HPPA_10
#  elif defined(_PA_RISC2_0) || defined(__RISC2_0__) || defined(__HPPA20__) || defined(__PA8000__)
#    define AX_ARCH_HPPA_10
#  endif
#endif

/* detect operation system */

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  include <TargetConditionals.h>
#  if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#    define AX_OS_DARWIN
#    define AX_OS_BSD4
#    define AX_OS_UNIX
#    ifdef __LP64__
#      define AX_OS_DARWIN64
#    else
#      define AX_OS_DARWIN32
#    endif
#    if defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#      if defined(TARGET_OS_WATCH) && TARGET_OS_WATCH
#        define AX_OS_WATCHOS
#      elif defined(TARGET_OS_TV) && TARGET_OS_TV
#        define AX_OS_TVOS
#      else
#        define AX_OS_IOS
#      endif
#    else
#      define AX_OS_MACOS
#    endif
#  else
#    error "AXE has not been ported to this Apple platform"
#  endif
#elif defined(__WEBOS__)
#  define AX_OS_WEBOS
#  define AX_OS_LINUX
#  define AX_OS_UNIX
#elif defined(__ANDROID__) || defined(ANDROID)
#  define AX_OS_ANDROID
#  define AX_OS_LINUX
#  define AX_OS_UNIX
#elif defined(__CYGWIN__)
#  define AX_OS_CYGWIN
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define AX_OS_WIN32
#  define AX_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#  if defined(WINAPI_FAMILY)
#    ifndef WINAPI_FAMILY_PC_APP
#      define WINAPI_FAMILY_PC_APP WINAPI_FAMILY_APP
#    endif
#    if defined(WINAPI_FAMILY_PHONE_APP) && WINAPI_FAMILY==WINAPI_FAMILY_PHONE_APP
#      define AX_OS_WINRT
#    elif WINAPI_FAMILY==WINAPI_FAMILY_PC_APP
#      define AX_OS_WINRT
#    else
#      define AX_OS_WIN32
#    endif
#  else
#    define AX_OS_WIN32
#  endif
#elif defined(__NetBSD__)
#  define AX_OS_NETBSD
#  define AX_OS_UNIX
#elif defined(__FreeBSD__)
#  define AX_OS_FREEBSD
#  define AX_OS_UNIX
#elif defined(unix) || defined(UNIX) || defined(__unix__) || defined(__UNIX__)
#  define AX_OS_UNIX
#endif

#endif
