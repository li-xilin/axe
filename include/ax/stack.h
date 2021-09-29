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

#ifndef AX_STACK_H
#define AX_STACK_H
#include "seq.h"
#include "tube.h"

#define AX_STACK_NAME AX_TUBE_NAME ".stack"

#ifndef AX_STACK_DEFINED
#define AX_STACK_DEFINED
typedef struct ax_stack_st ax_stack;
#endif

#define AX_CLASS_BASE_stack tube
#define AX_CLASS_ROLE_stack(_l) _l AX_CLASS_PTR(stack); AX_CLASS_ROLE_tube(_l)
AX_CLASS_STRUCT_ROLE(stack);

ax_tube *__ax_stack_construct(const ax_stuff_trait *elem_tr);

inline static AX_CLASS_CONSTRUCTOR(stack, const ax_stuff_trait *tr)
{
	return __ax_stack_construct(tr);
}

#endif
