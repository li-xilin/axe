/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_LOG_H
#define AX_LOG_H

#include "debug.h"
#include <stdarg.h>

#define AX_LL_DEBUG    0
#define AX_LL_INFO     1
#define AX_LL_WARN     2
#define AX_LL_ERROR    3
#define AX_LL_FATAL    4

#define AX_LM_NODEBUG (1 << 0)
#define AX_LM_NOINFO  (1 << 1)
#define AX_LM_NOWARN  (1 << 2)
#define AX_LM_NOERROR (1 << 3)
#define AX_LM_NOFATAL (1 << 4)
#define AX_LM_NOLOC   (1 << 5)
#define AX_LM_NOTIME  (1 << 6)

#define AX_LM_NOLOG \
	( AX_LM_NODEBUG \
	| AX_LM_NOINFO \
	| AX_LM_NOWARN \
	| AX_LM_NOERROR \
	| AX_LM_NOFATAL)

#define AX_LOG_MAX 1024

int __ax_log_print(const ax_location *loc, int level, const char* fmt, ...);
int __ax_log_vprint(const ax_location *loc, int level, const char* fmt, va_list ap);

void ax_log_set_mode(int mode);

int ax_log_mode();

typedef int ax_log_handler_f(const ax_location *loc, void *arg, int level, const char *text);

void ax_log_set_handler(ax_log_handler_f *f, void *arg);

#define ax_log(level, ...) __ax_log_print(AX_WHERE, level, __VA_ARGS__)

#define ax_pdebug(...) ax_log(AX_LL_DEBUG, __VA_ARGS__)
#define ax_pinfo(...) ax_log(AX_LL_INFO, __VA_ARGS__)
#define ax_pwarn(...) ax_log(AX_LL_WARN, __VA_ARGS__)
#define ax_perror(...) ax_log(AX_LL_ERROR, __VA_ARGS__)
#define ax_pfatal(...) ax_log(AX_LL_FATAL, __VA_ARGS__)

#endif

