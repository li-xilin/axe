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

#ifndef AX_UNIT_H
#define AX_UNIT_H
#include "debug.h"

typedef int ax_unit;

#define AX_UNIT_INITIALIZER 0

#ifndef NDEBUG

#define ax_unit_assert_ready(_uni) ((_uni) ? 0 : __ax_debug_assert_fail(__FILE__, __func__, __LINE__, \
		"uninitialized unit", "%s, aborted", #_uni))

#define __ax_unit_assert_not_ready(_uni) (!(_uni) ? 0 : __ax_debug_assert_fail(__FILE__, __func__, __LINE__, \
		"unit already initialized", "%s, aborted", #_uni))
#else

#define ax_unit_assert_ready(_uni) 0

#define __ax_unit_assert_not_ready(_uni) 0

#endif

#define ax_unit_init(_uni) if (!(_uni) || (_uni++, 0))

#define ax_unit_deinit(_uni) if (ax_unit_assert_ready(_uni), (_uni-- == 1))

#define ax_unit_ready(_uni) (void)(__ax_unit_assert_not_ready(_uni), (_uni = 1))

#define ax_unit_rc(_uni) (_uni)

#endif

