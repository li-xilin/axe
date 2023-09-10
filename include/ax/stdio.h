/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to one person obtaining a copy
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

#ifndef AX_STDIO_H
#define AX_STDIO_H

#include "detect.h"
#if defined(AX_OS_WIN) && defined(AX_CC_MINGW)
/* https://sourceforge.net/p/mingw-w64/bugs/846/ */
#  ifdef _INC_STDIO
#    error "ax/stdio.h should be placed before stdandard stdio.h"
#  else
#    undef __USE_MINGW_ANSI_STDIO
#    define __USE_MINGW_ANSI_STDIO 0
#  endif
#endif

#include "uchar.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>


#ifdef AX_OS_WIN
#define ax_fscanf(fp, fmt, ...)  fwscanf(fp, fmt, __VA_ARGS__)
#define ax_vfscanf(fp, fmt, ap) vfwscanf(fp, fmt, ap)
#define ax_fprintf(fp, fmt, ...) fwprintf(fp, fmt, __VA_ARGS__)
#define ax_vfprintf(fp, fmt, ap) vfwprintf(fp, fmt, ap)
#define ax_fopen(path, mode) _wfopen(path, mode)
#define ax_freopen(path, mode, fp) _wfreopen(path, mode, fp)
#else
#define ax_fscanf(fp, fmt, ...)  fscanf(fp, fmt, __VA_ARGS__)
#define ax_vfscanf(fp, fmt, ap) vfscanf(fp, fmt, ap)
#define ax_fprintf(fp, fmt, ...) fprintf(fp, fmt, __VA_ARGS__)
#define ax_vfprintf(fp, fmt, ap) vfprintf(fp, fmt, ap)
#define ax_fopen(path, mode) fopen(path, mode)
#define ax_freopen(path, mode, fp) freopen(path, mode, fp)
#endif

#define ax_scanf(fmt, ...) ax_fscanf(stdin, fmt, __VA_ARGS__)
#define ax_printf(fmt, ...) ax_fprintf(stdout, fmt, __VA_ARGS__)

FILE *ax_fdopen(intptr_t handle, const ax_uchar *mode);

#endif

