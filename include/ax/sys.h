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

#ifndef AX_SYS_H
#define AX_SYS_H

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
#    error "Axe has not been ported to this Apple platform"
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
#elif defined(VXWORKS) /* No "real" VxWorks define, must be set in the mkspec! */
#  define AX_OS_VXWORKS
#elif defined(__HAIKU__)
#  define AX_OS_HAIKU
#elif defined(__MAKEDEPEND__)
#else
#  error "Qt has not been ported to this OS - see http://www.qt-project.org/"
#endif

#if defined(AX_OS_WIN32) || defined(AX_OS_WIN64) || defined(AX_OS_WINRT)
#  define AX_OS_WINDOWS
#  define AX_OS_WIN
#endif

#if defined(AX_OS_WIN)
#  undef AX_OS_UNIX
#elif !defined(AX_OS_UNIX)
#  define AX_OS_UNIX
#endif

// Compatibility synonyms
#ifdef AX_OS_DARWIN
#define AX_OS_MAC
#endif
#ifdef AX_OS_DARWIN32
#define AX_OS_MAC32
#endif
#ifdef AX_OS_DARWIN64
#define AX_OS_MAC64
#endif
#ifdef AX_OS_MACOS
#define AX_OS_MACX
#define AX_OS_OSX
#endif

#ifdef AX_OS_DARWIN
#  include <Availability.h>
#  include <AvailabilityMacros.h>
#
#  ifdef AX_OS_MACOS
#    if !defined(__MAC_OS_X_VERSION_MIN_REQUIRED) || __MAC_OS_X_VERSION_MIN_REQUIRED < __MAC_10_6
#       undef __MAC_OS_X_VERSION_MIN_REQUIRED
#       define __MAC_OS_X_VERSION_MIN_REQUIRED __MAC_10_6
#    endif
#    if !defined(MAC_OS_X_VERSION_MIN_REQUIRED) || MAC_OS_X_VERSION_MIN_REQUIRED < MAC_OS_X_VERSION_10_6
#       undef MAC_OS_X_VERSION_MIN_REQUIRED
#       define MAC_OS_X_VERSION_MIN_REQUIRED MAC_OS_X_VERSION_10_6
#    endif
#  endif
#
#  // Numerical checks are preferred to named checks, but to be safe
#  // we define the missing version names in case Qt uses them.
#
#  if !defined(__MAC_10_11)
#       define __MAC_10_11 101100
#  endif
#  if !defined(__MAC_10_12)
#       define __MAC_10_12 101200
#  endif
#  if !defined(__MAC_10_13)
#       define __MAC_10_13 101300
#  endif
#  if !defined(__MAC_10_14)
#       define __MAC_10_14 101400
#  endif
#  if !defined(__MAC_10_15)
#       define __MAC_10_15 101500
#  endif
#  if !defined(__MAC_10_16)
#       define __MAC_10_16 101600
#  endif
#  if !defined(MAC_OS_X_VERSION_10_11)
#       define MAC_OS_X_VERSION_10_11 __MAC_10_11
#  endif
#  if !defined(MAC_OS_X_VERSION_10_12)
#       define MAC_OS_X_VERSION_10_12 __MAC_10_12
#  endif
#  if !defined(MAC_OS_X_VERSION_10_13)
#       define MAC_OS_X_VERSION_10_13 __MAC_10_13
#  endif
#  if !defined(MAC_OS_X_VERSION_10_14)
#       define MAC_OS_X_VERSION_10_14 __MAC_10_14
#  endif
#  if !defined(MAC_OS_X_VERSION_10_15)
#       define MAC_OS_X_VERSION_10_15 __MAC_10_15
#  endif
#  if !defined(MAC_OS_X_VERSION_10_16)
#       define MAC_OS_X_VERSION_10_16 __MAC_10_16
#  endif
#
#  if !defined(__IPHONE_10_0)
#       define __IPHONE_10_0 100000
#  endif
#  if !defined(__IPHONE_10_1)
#       define __IPHONE_10_1 100100
#  endif
#  if !defined(__IPHONE_10_2)
#       define __IPHONE_10_2 100200
#  endif
#  if !defined(__IPHONE_10_3)
#       define __IPHONE_10_3 100300
#  endif
#  if !defined(__IPHONE_11_0)
#       define __IPHONE_11_0 110000
#  endif
#  if !defined(__IPHONE_12_0)
#       define __IPHONE_12_0 120000
#  endif
#endif

#endif
