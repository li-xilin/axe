/*
 * Copyright (c) 2020-2023 Li Xilin <lixilin@gmx.com>
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

static int sg_mode = 0;
static ax_log_handler_f *sg_handler = NULL;
static void *sg_handler_arg = NULL;
	
void ax_log_set_handler(ax_log_handler_f *f, void *arg)
{
	sg_handler = f;
	sg_handler_arg = arg;
}

int ax_log_mode()
{
	return sg_mode;
}

void ax_log_set_mode(int mode)
{
	sg_mode = mode;
}

static int default_handler(const ax_location *loc, void *arg, int level, const char* text)
{
	char time_buf[64];
	char loc_buf[512];
	char level_buf[32];
	const char *type;

	FILE *fp = arg ? arg : stderr;

	switch(level)
	{
		case AX_LL_DEBUG:
			type = "DEBUG";
			break;
		case AX_LL_INFO:
			type = "INFO";
			break;
		case AX_LL_WARN:
			type = "WARN";
			break;
		case AX_LL_ERROR:
			type = "ERROR";
			break;
		case AX_LL_FATAL:
			type = "FATAL";
			break;
		default:
			sprintf(level_buf, "%d", level);
			type = level_buf;
	}

	time_buf[0] = '\0';
	loc_buf[0] = '\0';

	if (!(sg_mode & AX_LM_NOTIME)) {
		time_t tim = time(NULL);
		struct tm *t = localtime(&tim);
		sprintf(time_buf, "%4d-%02d-%02d %02d:%02d:%02d ", t->tm_year + 1900, t->tm_mon + 1,
				t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	}
	if (!(sg_mode & AX_LM_NOLOC)) {
		snprintf(loc_buf, sizeof loc_buf, "%s:%s:%d:", loc->file, loc->func, loc->line);
	}

	if (fprintf(fp, "[%-5s] %s%s%s\n", type, time_buf, loc_buf, text) < 0)
		return -1;

	if (fflush(fp))
		return -1;

	return 0;
}


int __ax_log_print(const ax_location *loc, int level, const char* fmt, ...)
{
	va_list va;
	int retval = -1;

	switch(level)
	{
		default:
		case AX_LL_DEBUG:
			if (sg_mode & AX_LM_NODEBUG)
				return 0;
			break;
		case AX_LL_INFO:
			if (sg_mode & AX_LM_NOINFO)
				return 0;
			break;
		case AX_LL_WARN:
			if (sg_mode & AX_LM_NOWARN)
				return 0;
			break;
		case AX_LL_ERROR:
			if (sg_mode & AX_LM_NOERROR)
				return 0;
			break;
		case AX_LL_FATAL:
			if (sg_mode & AX_LM_NOFATAL)
				return 0;
			break;
	}

	va_start(va, fmt);

	char msg_buf[AX_LOG_MAX];
	vsnprintf(msg_buf, sizeof msg_buf, fmt, va);

	retval = sg_handler
		? sg_handler(loc, sg_handler_arg, level, msg_buf)
		: default_handler(loc, sg_handler_arg, level, msg_buf);
	va_end(va);
	return retval;
}

