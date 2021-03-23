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

#include <axe/debug.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


void __ax_debug_assert_fail(const char* file, const char* func, int line, const char* brief, const char* fmt, ...)
{
	va_list vl;
	char text[2048];
	int nchar = snprintf(text, sizeof text, "%s:%s:%d:%s", file, func, line, brief);
	if (fmt && sizeof text - nchar > sizeof ":\n") {
		text[nchar++] = ':';
		text[nchar] = '\0';
		va_start(vl, fmt);
		nchar += vsnprintf(text + nchar, sizeof text - nchar, fmt, vl);
		va_end(vl);
	} 
	text[nchar++] = '\n';
	text[nchar]= '\0';
	fputs(text, stderr);
		
	abort();
}
