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

#define ax_block for(register int __ax_block_flag = 0; __ax_block_flag != 1; __ax_block_flag = 1)

#define ax_repeat(_n) for(size_t _ = 0; _ != (_n); _++)

#define ax_forever for(;;)

#define ax_forvalues(_t, ...) \
	for (int64_t __ax_i = 1; __ax_i <= AX_NARG_T(_t, __VA_ARGS__); __ax_i = - __ax_i, __ax_i++) \
		for (_t _ = ((_t[]){ __VA_ARGS__ })[__ax_i - 1]; __ax_i > 0; __ax_i = - __ax_i)

#define __AX_FORRANGE_STEP(...) \
	(AX_NARG_T(ptrdiff_t, __VA_ARGS__) > 1 ? ax_ptrof(ptrdiff_t, __VA_ARGS__)[1] : 1)

#define __AX_FORRANGE_LAST(...) \
	ax_ptrof(ptrdiff_t, __VA_ARGS__)[0]

#define __AX_FORRANGE_END(first, ...) \
	(__AX_FORRANGE_LAST(__VA_ARGS__) \
	 - (__AX_FORRANGE_LAST(__VA_ARGS__) - first - 1) % __AX_FORRANGE_STEP(__VA_ARGS__) \
	 + __AX_FORRANGE_STEP(__VA_ARGS__)) - 1

#define ax_forrange(first, ...) \
	for (ptrdiff_t _ = first, __ax_forrange_end = __AX_FORRANGE_END(first, __VA_ARGS__); \
		_ != __ax_forrange_end; \
		_ += __AX_FORRANGE_STEP(__VA_ARGS__))

#define __AX_SEGMENT_LABEL_ENTER(name, n) __ax_segment_enter_##name##_##n
#define __AX_SEGMENT_LABEL_LEAVE(name, n) __ax_segment_leave_##name##_##n
#define __AX_SEGMENT_CONTEXT_NAME(name) struct __ax_segment_context_##name

#define __AX_SEGMENT_CASE(n, name) \
		__AX_SEGMENT_LABEL_ENTER(name, n): \
			if ( __ax_segment_no == n || (!__ax_segment_no && ((__ax_segment_no = n), 0)) ) { \
				__ax_segment_no=0; goto __AX_SEGMENT_LABEL_LEAVE(name, n); \
			} else

#define ax_segment(name, n) \
	for (register int __ax_segment_no = 0; __ax_segment_no ; ) \
		AX_PAVE_TO(n, __AX_SEGMENT_CASE, name)

#define ax_segment_do(name, n) \
	goto __AX_SEGMENT_LABEL_ENTER(name, n); \
	__AX_SEGMENT_LABEL_LEAVE(name, n):;

#endif
