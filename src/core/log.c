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
	

int ax_log_set_fp(void *fp)
{
	if (setvbuf(fp, 0, _IONBF, 0))
		return -1;
	g_logfp = fp;
	return 0;
}

void *ax_log_fp()
{
	return g_logfp ? g_logfp: stderr;
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
	char msg_buf[AX_LOG_MAX_LEN];
	char time_buf[64];
	char loc_buf[512];
	char level_buf[32];
	const char *type;

	FILE *fp = g_logfp ? g_logfp : stderr;

	va_list vl;
	va_start(vl, fmt);
	vsnprintf(msg_buf, sizeof msg_buf, fmt, vl);
	va_end(vl);

	switch(level)
	{
		case AX_LL_DEBUG:
			if (g_mode & AX_LM_NODEBUG)
				return 0;
			type = "DEBUG";
			break;
		case AX_LL_INFO:
			if (g_mode & AX_LM_NOINFO)
				return 0;
			type = "INFO";
			break;
		case AX_LL_WARN:
			if (g_mode & AX_LM_NOWARN)
				return 0;
			type = "WARN";
			break;
		case AX_LL_ERROR:
			if (g_mode & AX_LM_NOERROR)
				return 0;
			type = "ERROR";
			break;
		case AX_LL_FATAL:
			if (g_mode & AX_LM_NOFATAL)
				return 0;
			type = "FATAL";
			break;
		default:
			sprintf(level_buf, "%d", level);
			type = level_buf;
	}

	time_buf[0] = '\0';
	loc_buf[0] = '\0';

	if (!(g_mode & AX_LM_NOTIME)) {
		time_t tim = time(NULL);
		struct tm *t = localtime(&tim);
		sprintf(time_buf, "%4d-%02d-%02d %02d:%02d:%02d ", t->tm_year + 1900, t->tm_mon + 1,
				t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	}
	if (!(g_mode & AX_LM_NOLOC)) {
		snprintf(loc_buf, sizeof loc_buf, "%s:%s:%d:", file, func, line);
	}

	int ret;
	ret = fprintf(fp,
			"[%-5s] %s%s%s\n",
			type, time_buf, loc_buf, msg_buf) < 0 ? -1 : 0;

	fflush(fp);
	return ret;
}

