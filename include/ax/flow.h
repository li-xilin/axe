/*
 * Copyright (c) 2022 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_FLOW_H
#define AX_FLOW_H

#include "def.h"
#include "trick.h"
#include "narg.h"
#include <stddef.h>
#include <stdint.h>
#include <setjmp.h>

#define ax_block for(register int __ax_block_flag = 0; __ax_block_flag != 1; __ax_block_flag = 1)

#define ax_repeat(_n) for(size_t _ = 0; _ != (_n); _++)

#define ax_forever for(;;)

#define ax_forvalues(_t, ...) \
	for (int64_t __ax_i = 1; __ax_i <= AX_NARG_T(_t, __VA_ARGS__); __ax_i = - __ax_i, __ax_i++) \
		for (_t _ = ((_t[]){ __VA_ARGS__ })[__ax_i - 1]; __ax_i > 0; __ax_i = - __ax_i)

#define __AX_FORRANGE_STEP(...) \
	(AX_NARG_T(ptrdiff_t, __VA_ARGS__) > 1 ? ax_p(ptrdiff_t, __VA_ARGS__)[1] : 1)

#define __AX_FORRANGE_LAST(...) \
	ax_p(ptrdiff_t, __VA_ARGS__)[0]

#define __AX_FORRANGE_END(first, ...) \
	(__AX_FORRANGE_LAST(__VA_ARGS__) \
	 - (__AX_FORRANGE_LAST(__VA_ARGS__) - first + (__AX_FORRANGE_STEP(__VA_ARGS__) > 0 ? -1 : 1)) % __AX_FORRANGE_STEP(__VA_ARGS__) \
	 + __AX_FORRANGE_STEP(__VA_ARGS__)) + (__AX_FORRANGE_STEP(__VA_ARGS__) > 0 ? -1 : 1)

#define ax_forrange(first, ...) \
	for (ptrdiff_t _ = first, __ax_forrange_end = __AX_FORRANGE_END(first, __VA_ARGS__); \
		_ != __ax_forrange_end; \
		_ += __AX_FORRANGE_STEP(__VA_ARGS__))

#define ax_routine(name) jmp_buf __ax_routine_jmpbuf_##name; while (0) __ax_routine_label_##name: \
                for (int __ax_routine_flag = 0;  ; __ax_routine_flag = 1) \
		if (__ax_routine_flag) longjmp(__ax_routine_jmpbuf_##name, 1); else \
                for (; !__ax_routine_flag ; __ax_routine_flag = 1)

#define ax_routine_call(name) if (!setjmp(__ax_routine_jmpbuf_##name)) goto __ax_routine_label_##name;

#endif
