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

#include <axe/error.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* ax_error_str(int errno)
{
	switch(errno) {
		case AX_ERR_SUCCEED: return "Succeed"; break;
		case AX_ERR_NOMEM:   return "Memory alloc failed"; break;
		case AX_ERR_EMPTY:   return "Container is empty"; break;
		case AX_ERR_FULL:    return "Container is full"; break;
		case AX_ERR_NOKEY:   return "Key is not exists"; break;
		case AX_ERR_UNSUP:   return "Unsupported operation"; break;
		case AX_ERR_ITACC:   return "Where iterator point to can not be accessed"; break;
		case AX_ERR_BADIT:   return "Invalid iterator"; break;
		case AX_ERR_OVERFLOW:return "Access overflow"; break;
	}
	return "Unknow errno";
}

