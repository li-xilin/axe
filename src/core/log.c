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

#include "ax/log.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

static FILE *g_logfp = NULL;
static int g_mode = 0;
	

void ax_log_set_fp(void *fp)
{
	g_logfp = fp;
}

void *ax_log_fp()
{
	return g_logfp;
}

int ax_log_mode()
{
	return g_mode;
}

void ax_log_set_mode(int mode)
{
	g_mode = mode;
}

int __ax_log_print(const char *file, const char *func, int line, int level, const char* fmt, ...)
{

	static const char *type_debug = "DEBUG";
	static const char *type_info = "INFO";
	static const char *type_warn= "WARN";
	static const char *type_error = "ERR";
	static const char *type_fatal = "DEBUG";

	const char* type;
	switch(level)
	{
		case AX_LL_DEBUG:
			if (g_mode & AX_LM_NODEBUG)
				return 0;
			type = type_debug;
			break;
		case AX_LL_INFO:
			if (g_mode & AX_LM_NOINFO)
				return 0;
			type = type_info;
			break;
		case AX_LL_WARN:
			if (g_mode & AX_LM_NOWARN)
				return 0;
			type = type_warn;
			break;
		case AX_LL_ERROR:
			if (g_mode & AX_LM_NOERROR)
				return 0;
			type = type_error;
			break;
		case AX_LL_FATAL:
			if (g_mode & AX_LM_NOFATAL)
				return 0;
			type = type_fatal;
			break;
		default:
			errno = EINVAL;
			return -1;
	}

	char buf[AX_LOG_MAX_LEN];
	buf[sizeof buf - 1] = '\0';

	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buf, sizeof buf - 1, fmt, vl);
	va_end(vl);

	char time_buf[64];
	time_t tim = time(NULL);
	struct tm *t = localtime(&tim);
	sprintf(time_buf, "%4d-%02d-%02d %02d:%02d:%02d:%03ld", t->tm_year + 1900, t->tm_mon + 1,
			t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec, clock() % 1000);


	int ret = fprintf(g_logfp ? g_logfp : stderr,
			"[%-4s] %s %s:%s:%d:%s\n",
			type, time_buf, file, func, line, buf) == -1
		? -1
		: 0;
	fflush(g_logfp);
	return ret;
}

