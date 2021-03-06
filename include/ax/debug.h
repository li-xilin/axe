/*
 * Copyright (c) 2020-2021 Li hsilin <lihsilyn@gmail.com>
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

int __ax_debug_assert_fail (const char* file, const char* func,
		int line, const char* brief, const char* fmt, ...);

#ifndef NDEBUG
# define ax_assert(_exp, ...) ((_exp) \
	? (void)0 : (void)__ax_debug_assert_fail(__FILE__, \
		__func__, __LINE__, "assertion failed", __VA_ARGS__))
#else
# define ax_assert(_exp, ...) ((void)0)
#endif

#define ax_assert_not_null(x) ax_assert((x), "unexpected NULL with `%s`", #x);

#endif

