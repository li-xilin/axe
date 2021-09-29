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

#ifndef AX_STRING_H
#define AX_STRING_H
#include "str.h"
#include "debug.h"

#define AX_STRING_NAME AX_STR_NAME ".string"

#ifndef AX_STRING_DEFINED
#define AX_STRING_DEFINED
typedef struct ax_string_st ax_string;
#endif

#define AX_CLASS_BASE_string str
#define AX_CLASS_ROLE_string(_l) _l AX_CLASS_PTR(string); AX_CLASS_ROLE_str(_l)
AX_CLASS_STRUCT_ROLE(string);

ax_str *__ax_string_construct();

inline static AX_CLASS_CONSTRUCTOR0(string)
{
	return __ax_string_construct();
}

#endif
