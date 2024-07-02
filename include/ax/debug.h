/*
 * Copyright (c) 2020-2021, 2023-2024 Li Xilin <lixilin@gmx.com>
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

#ifndef AX_DEBUG_H
#define AX_DEBUG_H

#include "trick.h"
#include "narg.h"

typedef struct {
        const char *file;
        const char *func;
        int line;
} ax_location;

#define AX_WHERE ((const ax_location[1]) { { __FILE__, __func__, __LINE__ } })

int __ax_debug_assert_fail (const ax_location *loc, const char* brief, const char* fmt, ...);

#ifndef NDEBUG
# define ax_assert(_exp, ...) ((_exp) \
	? (void)0 : (void)__ax_debug_assert_fail(AX_WHERE, "assertion failed", __VA_ARGS__))
#else
# define ax_assert(_exp, ...) ((void)0)
#endif

#ifndef NDEBUG
# define ax_static_assert(_exp) typedef char AX_CATENATE(__ax_static_assert_, \
		__LINE__)[(_exp) ? 1 : -1]
#else
# define ax_static_assert(_exp) typedef char AX_CATENATE(__ax_static_assert_, __LINE__)[1]
#endif

#define ax_assert_not_null(x) ax_assert((x), "unexpected NULL value for `%s`", #x);

#endif

