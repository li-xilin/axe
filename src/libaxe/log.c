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

#include <axe/log.h>

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *type_info = "[INFO]";
static const char *type_warn= "[WARN]";
static const char *type_err= "[ERR]";

void __ax_log_print(int level, const char* fmt, ...)
{
	va_list vl;
	char type_buffer[32];
	const char* type;
	switch(level)
	{
		case AX_LM_INFO:
			type = type_info;
			break;
		case AX_LM_WARNING:
			type = type_warn;
			break;
		case AX_LM_ERROR:
			type = type_err;
			break;
		default:
			sprintf(type_buffer, "[LOG %d]", level);
			type = type_buffer;
			break;
	}

	fprintf(stdout, "%s ", type);

	va_start(vl, fmt);
	vfprintf(stdout, fmt, vl);
	va_end(vl);

	fputc('\n', stdout);
}
