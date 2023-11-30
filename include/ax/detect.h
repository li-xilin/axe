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

/* Detect machine byte-order */

#if defined(__BYTE_ORDER__)
#  define AX_BYTE_ORDER __BYTE_ORDER__
#elif defined(__BYTE_ORDER)
#  define AX_BYTE_ORDER __BYTE_ORDER
#elif defined(BYTE_ORDER)
#  define AX_BYTE_ORDER BYTE_ORDER
#else
#  error "Unknown byte-order"
#endif

#if defined(__ORDER_BIG_ENDIAN__)
#  define AX_BIG_ENDIAN __ORDER_BIG_ENDIAN__
#elif defined(__BIG_ENDIAN)
#  define AX_BIG_ENDIAN __BIG_ENDIAN
#else
#  define AX_BIG_ENDIAN 4321
#endif

#if defined(__ORDER_LITTLE_ENDIAN__)
#  define AX_LITTLE_ENDIAN __ORDER_LITTLE_ENDIAN__
#elif defined(__LITTLE_ENDIAN)
#  define AX_LITTLE_ENDIAN __LITTLE_ENDIAN
#else
#  define AX_LITTLE_ENDIAN 1234
#endif

#if defined(__ORDER_PDP_ENDIAN__)
#  define AX_PDP_ENDIAN __ORDER_PDP_ENDIAN__
#elif defined(__ORDER_PDP_ENDIAN)
#  define AX_PDP_ENDIAN __ORDER_PDP_ENDIAN
#else
#  define AX_PDP_ENDIAN 3412
#endif

/* Detect architecture */

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
#elif defined(__PPC64__) || defined(__ppc64__) || defined(__ppc64) || defined(__powerpc64__) || defined(_ARCH_PPC64)
#  define AX_ARCH_PPC64
#elif defined(__PPC__) || defined(__ppc__) || defined(__ppc) || defined(__powerpc__) || defined(__POWERPC__) || defined(_M_PPC) || defined(_ARCH_PPC)
#  define AX_ARCH_PPC
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

/*
   The operating system, must be one of: (AX_OS_x)
     DARWIN   - Any Darwin system (macOS, iOS, watchOS, tvOS)
     MACOS    - macOS
     IOS      - iOS
     WATCHOS  - watchOS
     TVOS     - tvOS
     WIN32    - Win32 (Windows 2000/XP/Vista/7 and Windows Server 2003/2008)
     CYGWIN   - Cygwin
     SOLARIS  - Sun Solaris
     HPUX     - HP-UX
     LINUX    - Linux [has variants]
     FREEBSD  - FreeBSD [has variants]
     NETBSD   - NetBSD
     OPENBSD  - OpenBSD
     INTERIX  - Interix
     AIX      - AIX
     HURD     - GNU Hurd
     QNX      - QNX [has variants]
     QNX6     - QNX RTP 6.1
     LYNX     - LynxOS
     BSD4     - Any BSD 4.4 system
     UNIX     - Any UNIX BSD/SYSV system
     ANDROID  - Android platform
     HAIKU    - Haiku
     WEBOS    - LG WebOS
   The following operating systems have variants:
     LINUX    - both AX_OS_LINUX and AX_OS_ANDROID are defined when building for Android
              - only AX_OS_LINUX is defined if building for other Linux systems
     MACOS    - both AX_OS_BSD4 and AX_OS_IOS are defined when building for iOS
              - both AX_OS_BSD4 and AX_OS_MACOS are defined when building for macOS
     FREEBSD  - AX_OS_FREEBSD is defined only when building for FreeBSD with a BSD userland
              - AX_OS_FREEBSD_KERNEL is always defined on FreeBSD, even if the userland is from GNU
*/

#if defined(__APPLE__) && (defined(__GNUC__) || defined(__xlC__) || defined(__xlc__))
#  include <TargetConditionals.h>
#  if defined(TARGET_OS_MAC) && TARGET_OS_MAC
#    define AX_OS_DARWIN
#    define AX_OS_BSD4
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
#    error "Unknown system"
#  endif
#elif defined(__WEBOS__)
#  define AX_OS_WEBOS
#  define AX_OS_LINUX
#elif defined(__ANDROID__) || defined(ANDROID)
#  define AX_OS_ANDROID
#  define AX_OS_LINUX
#elif defined(__CYGWIN__)
#  define AX_OS_CYGWIN
#elif !defined(SAG_COM) && (!defined(WINAPI_FAMILY) || WINAPI_FAMILY==WINAPI_FAMILY_DESKTOP_APP) && (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))
#  define AX_OS_WIN32
#  define AX_OS_WIN64
#elif !defined(SAG_COM) && (defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__))
#    define AX_OS_WIN32
#elif defined(__sun) || defined(sun)
#  define AX_OS_SOLARIS
#elif defined(hpux) || defined(__hpux)
#  define AX_OS_HPUX
#elif defined(__native_client__)
#  define AX_OS_NACL
#elif defined(__EMSCRIPTEN__)
#  define AX_OS_WASM
#elif defined(__linux__) || defined(__linux)
#  define AX_OS_LINUX
#elif defined(__FreeBSD__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__)
#  ifndef __FreeBSD_kernel__
#    define AX_OS_FREEBSD
#  endif
#  define AX_OS_FREEBSD_KERNEL
#  define AX_OS_BSD4
#elif defined(__NetBSD__)
#  define AX_OS_NETBSD
#  define AX_OS_BSD4
#elif defined(__OpenBSD__)
#  define AX_OS_OPENBSD
#  define AX_OS_BSD4
#elif defined(__INTERIX)
#  define AX_OS_INTERIX
#  define AX_OS_BSD4
#elif defined(_AIX)
#  define AX_OS_AIX
#elif defined(__Lynx__)
#  define AX_OS_LYNX
#elif defined(__GNU__)
#  define AX_OS_HURD
#elif defined(__QNXNTO__)
#  define AX_OS_QNX
#elif defined(__INTEGRITY)
#  define AX_OS_INTEGRITY
#elif defined(__rtems__)
#  define AX_OS_RTEMS
#elif defined(__HAIKU__)
#  define AX_OS_HAIKU
#else
#  error "Unknown system"
#endif

#if defined(AX_OS_WIN32) || defined(AX_OS_WIN64)
#  define AX_OS_WIN
#endif

#if defined(AX_OS_WIN)
#  undef AX_OS_UNIX
#elif !defined(AX_OS_UNIX)
#  define AX_OS_UNIX
#endif

#if defined(AX_OS_DARWIN)
#    define AX_OS_TEXT "darwin"
#  elif defined(AX_OS_MACOS)
#    define AX_OS_TEXT "macos"
#  elif defined(AX_OS_IOS)
#    define AX_OS_TEXT "ios"
#  elif defined(AX_OS_WATCHOS)
#    define AX_OS_TEXT "watchos"
#  elif defined(AX_OS_TVOS)
#    define AX_OS_TEXT "tvos"
#  elif defined(AX_OS_WIN32)
#    define AX_OS_TEXT "win32"
#  elif defined(AX_OS_CYGWIN)
#    define AX_OS_TEXT "cygwin"
#  elif defined(AX_OS_SOLARIS)
#    define AX_OS_TEXT "solaris"
#  elif defined(AX_OS_HPUX)
#    define AX_OS_TEXT "hpux"
#  elif defined(AX_OS_LINUX)
#    define AX_OS_TEXT "linux"
#  elif defined(AX_OS_FREEBSD)
#    define AX_OS_TEXT "freebsd"
#  elif defined(AX_OS_NETBSD)
#    define AX_OS_TEXT "netbsd"
#  elif defined(AX_OS_OPENBSD)
#    define AX_OS_TEXT "openbsd"
#  elif defined(AX_OS_INTERIX)
#    define AX_OS_TEXT "interix"
#  elif defined(AX_OS_AIX)
#    define AX_OS_TEXT "aix"
#  elif defined(AX_OS_HURD)
#    define AX_OS_TEXT "qnx"
#  elif defined(AX_OS_QNX)
#    define AX_OS_TEXT "qnx"
#  elif defined(AX_OS_QNX6)
#    define AX_OS_TEXT "qnx6"
#  elif defined(AX_OS_LYNX)
#    define AX_OS_TEXT "lynx"
#  elif defined(AX_OS_BSD4)
#    define AX_OS_TEXT "bsd4"
#  elif defined(AX_OS_ANDROID)
#    define AX_OS_TEXT "android"
#  elif defined(AX_OS_HAIKU)
#    define AX_OS_TEXT "haiku"
#  elif defined(AX_OS_WEBOS)
#    define AX_OS_TEXT "webos"
#  elif defined(AX_OS_UNIX)
#    define AX_OS_TEXT "unix"
#  else
#    define AX_OS_TEXT "unknown"
#endif

/* Detect compiler */

#if defined(_MSC_VER)
#  define AX_CC_MSVC
#  if defined(__INTEL_COMPILER)
#    define AX_CC_INTEL
#  endif
#elif defined(__BORLANDC__) || defined(__TURBOC__)
#  define AX_CC_BORLANDC
#elif defined(__WATCOMC__)
#  define AX_CC_WATCOMC
/* Symbian GCCE */
#elif defined(__GCCE__)
#  define AX_CC_GCCE
/* RVCT compiler also defines __EDG__ and __GNUC__ , so check for it before that */
#elif defined(__ARMCC__) || defined(__CC_ARM)
#  define AX_CC_RVCT
#elif defined(__GNUC__)
#  define AX_CC_GNU
#  if defined(__MINGW32__)
#    define AX_CC_MINGW
#  endif
#  if defined(__INTEL_COMPILER)
/* Intel C++ also masquerades as GCC 3.2.0 */
#    define AX_CC_INTEL
#  endif
#  if defined(__clang__)
/* Clang also masquerades as GCC 4.2.1 */
#    define AX_CC_CLANG
#  endif
#endif
#elif defined(__xlC__)
#  define AX_CC_XLC
#elif defined(__DECCXX) || defined(__DECC)
#  define AX_CC_DEC
#  if defined(__EDG__)
#    define AX_CC_EDG
#  endif
/* The Portland Group C++ compiler is based on EDG and does define __EDG__
   but the C compiler does not */
#elif defined(__PGI)
#  define AX_CC_PGI
#elif !defined(AX_OS_HPUX) && (defined(__EDG) || defined(__EDG__))
#  define AX_CC_EDG
#  if defined(__COMO__)
#    define AX_CC_COMEAU
#  elif defined(__INTEL_COMPILER)
#    define AX_CC_INTEL
#  elif defined(__DCC__)
#    define AX_CC_DIAB
#  elif defined(__USLC__) && defined(__SCO_VERSION__)
#    define AX_CC_USLC
#  elif defined(CENTERLINE_CLPP) || defined(OBJECTCENTER)
#    define AX_CC_OC
#elif defined(_DIAB_TOOL)
#  define AX_CC_DIAB
#elif defined(__HIGHC__)
#  define AX_CC_HIGHC
#elif defined(__SUNPRO_CC) || defined(__SUNPRO_C)
#  define AX_CC_SUN
#elif defined(sinix)
#  define AX_CC_EDG
#  define AX_CC_CDS
#elif defined(AX_OS_HPUX)
#  define AX_CC_HPACC
#elif defined(__WINSCW__) && !defined(AX_CC_NOKIAX86)
#  define AX_CC_NOKIAX86
#elif defined(__TINYC__)
#  define AX_CC_TCC
#elif defined(__PCC__)
#  define AX_CC_PCC
#else
#  error "Unknown compiler"
#endif

#endif
