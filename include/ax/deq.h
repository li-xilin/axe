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

#ifndef AX_DEQ_H
#define AX_DEQ_H
#include "seq.h"

#define AX_DEQ_NAME AX_SEQ_NAME ".deq"

#ifndef AX_DEQ_DEFINED
#define AX_DEQ_DEFINED
typedef struct ax_deq_st ax_deq;
#endif

#define AX_CLASS_BASE_deq seq
#define AX_CLASS_ROLE_deq(_l) _l AX_CLASS_PTR(deq); AX_CLASS_ROLE_seq(_l)

AX_CLASS_STRUCT_ROLE(deq);

extern const ax_seq_trait ax_deq_tr;

ax_seq *__ax_deq_construct(const ax_trait *elem_tr);

inline static AX_CLASS_CONSTRUCTOR(deq, const ax_trait* trait)
{
	return __ax_deq_construct(trait);
}

#endif
