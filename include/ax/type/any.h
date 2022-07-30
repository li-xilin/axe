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

#ifndef AX_ANY_H
#define AX_ANY_H
#include "one.h"

#ifndef AX_DUMP_DEFINED
#define AX_DUMP_DEFINED
typedef struct ax_dump_st ax_dump;
#endif

#ifndef AX_ANY_DEFINED
#define AX_ANY_DEFINED
typedef struct ax_any_st ax_any;
#endif

#define ax_baseof_ax_any ax_one

ax_abstract_env_begin(ax_any)
ax_end;

ax_abstract_begin(ax_any)
	ax_any *(*copy)(const ax_any* any);
	ax_dump *(*dump)(const ax_any* any);
ax_end;

ax_abstract(1, ax_any);

inline static ax_any *ax_any_copy(const ax_any* any)
{
	return ax_obj_do0(any, copy);
}

inline static ax_dump *ax_any_dump(const ax_any* any)
{
	return ax_obj_do0(any, dump);
}

ax_fail __ax_any_print(const ax_any *any, const char *file, int line);

#define ax_any_so(_any) __ax_any_print(_any, __FILE__, __LINE__)

//extern const ax_trait ax_any_tr;

#endif

