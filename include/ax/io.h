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

#ifndef AX_IO_H
#define AX_IO_H

#include "detect.h"
#if defined(AX_OS_WIN) && defined(AX_CC_MINGW)
/* https://sourceforge.net/p/mingw-w64/bugs/846/ */
#  ifdef _INC_STDIO
#    error "ax/io.h should be placed before stdandard stdio.h"
#  else
#    undef __USE_MINGW_ANSI_STDIO
#    define __USE_MINGW_ANSI_STDIO 0
#  endif
#endif

#include "uchar.h"
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

int ax_io_setinput(FILE *stream);
int ax_io_setoutput(FILE *stream);
FILE* ax_fdopen(intptr_t handle, const ax_uchar *mode);

static inline FILE *ax_fopen(const ax_uchar *path, const ax_uchar *mode)
{
#ifdef AX_OS_WIN
	return _wfopen(path, mode);
#else
	return fopen(path, mode);
#endif
}

static inline FILE *ax_freopen(const ax_uchar *path, const ax_uchar *mode, FILE *stream)
{
#ifdef AX_OS_WIN
	return _wfreopen(path, mode, stream);
#else
	return freopen(path, mode, stream);
#endif
}

static inline int ax_vfprintf(FILE *stream, const ax_uchar *format, va_list ap)
{
#ifdef AX_OS_WIN
	return vfwprintf(stream, format, ap);
#else
	return vfprintf(stream, format, ap);
#endif
}

static inline int ax_fprintf(FILE *stream, const ax_uchar *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int ret = ax_vfprintf(stream, format, ap);
	va_end(ap);
	return ret;
}


static inline int ax_vprintf(const ax_uchar *format, va_list ap)
{
#ifdef AX_OS_WIN
	return vwprintf(format, ap);
#else
	return vprintf(format, ap);
#endif
}

static inline int ax_printf(const ax_uchar *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int ret = ax_vprintf(format, ap);
	va_end(ap);
	return ret;
}

static inline int ax_fputs(const ax_uchar *ws, FILE *stream)
{
#ifdef AX_OS_WIN
	return fputws(ws, stream);
#else
	return fputs(ws, stream);
#endif
}

static inline wint_t ax_fputc(ax_uchar c, FILE *stream)
{
#ifdef AX_OS_WIN
	return fputwc(c, stream);
#else
	return fputc(c, stream);
#endif
}

static inline wint_t ax_putchar(ax_uchar c)
{
#ifdef AX_OS_WIN
	return fputwc(c, stdout);
#else
	return fputc(c, stdout);
#endif
}

static inline int ax_vfscanf(FILE *stream, const ax_uchar *format, va_list ap)
{
#ifdef AX_OS_WIN
	return vfwscanf(stream, format, ap);
#else
	return vfscanf(stream, format, ap);
#endif
}

static inline int ax_fscanf(FILE *stream, const ax_uchar *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int ret = ax_vfscanf(stream, format, ap);
	va_end(ap);
	return ret;
}

static inline int ax_vscanf(const ax_uchar *format, va_list ap)
{
#ifdef AX_OS_WIN
	return vwscanf(format, ap);
#else
	return vscanf(format, ap);
#endif
}

static inline int ax_scanf(const ax_uchar *format, ...)
{
	va_list ap;
	va_start(ap, format);
	int ret = ax_vscanf(format, ap);
	va_end(ap);
	return ret;
}

static inline ax_uchar *ax_fgets(ax_uchar *ws, int n, FILE *stream)
{
#ifdef AX_OS_WIN
	return fgetws(ws, n, stream);
#else
	return fgets(ws, n, stream);
#endif
}

static inline wint_t ax_fgetc(FILE *stream)
{
#ifdef AX_OS_WIN
	return fgetwc(stream);
#else
	return fgetc(stream);
#endif
}

static inline int ax_getchar(void)
{
#ifdef AX_OS_WIN
	return getwchar();
#else
	return getchar();
#endif
}

#endif

