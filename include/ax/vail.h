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

#ifndef AX_VAIL_H
#define AX_VAIL_H
#include "debug.h"
#include <stdint.h>
#include <stdarg.h>

#ifndef AX_VAIL_DEFINED
#define AX_VAIL_DEFINED
typedef struct ax_vail_st ax_vail;
#endif

struct ax_vail_info_st
{
	void  *ptr;
	size_t size;
	int    type;
};
typedef struct ax_vail_info_st ax_vail_info;

ax_vail *ax_vail_vcreate(const char *fmt, va_list valist);

ax_vail *ax_vail_create(const char *fmt, ...);

void ax_vail_get(ax_vail *vail, uint8_t idx, ax_vail_info *info);

size_t ax_vail_size(ax_vail *vail);

void ax_vail_destroy(ax_vail *vail);

#endif
