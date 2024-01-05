/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
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

#ifndef AX_LOG2_H
#define AX_LOG2_H

#include "log.h"
#include "uchar.h"

int __ax_log2_print(const ax_location *loc, int level, const ax_uchar* fmt, ...);
int __ax_log2_vprint(const ax_location *loc, int level, const ax_uchar* fmt, va_list ap);

typedef int ax_log2_handler_f(const ax_location *loc, void *arg, int level, const ax_uchar *text);
void ax_log2_set_handler(ax_log2_handler_f *f, void *arg);

#undef ax_log
#define ax_log(level, ...) __ax_log2_print(AX_WHERE, level, ax_u("") __VA_ARGS__)

#undef ax_pdebug
#define ax_pdebug(...) ax_log(AX_LL_DEBUG, __VA_ARGS__)

#undef ax_pinfo
#define ax_pinfo(...) ax_log(AX_LL_INFO, __VA_ARGS__)

#undef ax_pwarn
#define ax_pwarn(...) ax_log(AX_LL_WARN, __VA_ARGS__)

#undef ax_perror
#define ax_perror(...) ax_log(AX_LL_ERROR, __VA_ARGS__)

#undef ax_pfatal
#define ax_pfatal(...) ax_log(AX_LL_FATAL, __VA_ARGS__)

#endif

