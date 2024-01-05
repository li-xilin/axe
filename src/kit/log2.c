/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
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

#include "ax/log2.h"
#include "ax/io.h"
#include "ax/mutex.h"
#include "ax/once.h"
#include "ax/tcolor.h"
#include "ax/path.h"
#include "ax/debug.h"

#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <time.h>

static ax_log2_handler_f *s_handler = NULL;
static void *s_handler_arg = NULL;
static ax_mutex s_lock = AX_MUTEX_INIT;

void ax_log2_set_handler(ax_log2_handler_f *f, void *arg)
{
	s_handler = f;
	s_handler_arg = arg;
}

static int log2_default_handler(const ax_location *loc, void *arg, int level, const ax_uchar* text)
{
	int retval = -1;
	ax_uchar time_buf[64];
	ax_uchar loc_buf[512];
	ax_uchar level_buf[32];
	ax_uchar file_buf[AX_PATH_MAX];
	ax_uchar func_buf[128];
	const ax_uchar *type;
	int mode = ax_log_mode();
	FILE *fp = arg ? arg : stderr;

	time_buf[0] = '\0';
	loc_buf[0] = '\0';

	if (!(mode & AX_LM_NOTIME)) {
		time_t tim = time(NULL);
		struct tm *t = localtime(&tim);
		ax_usnprintf(time_buf, ax_nelems(time_buf), ax_u("%4d-%02d-%02d %02d:%02d:%02d"),
				t->tm_year + 1900, t->tm_mon + 1,
				t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	}

	if (!(mode & AX_LM_NOLOC)) {
		ax_ustr_from_utf8(file_buf, ax_nelems(file_buf), loc->file);
		ax_ustr_from_utf8(func_buf, ax_nelems(func_buf), loc->func);
		ax_usnprintf(loc_buf, sizeof loc_buf, ax_u("%" AX_PRIus ":%" AX_PRIus ":%d"), file_buf, func_buf, loc->line);
	}

	if (ax_mutex_lock(&s_lock))
		goto out;

	if (ax_fputc(ax_u('['), fp) < 0)
		goto out;

	ax_tcolor_set(fp);
	switch(level) {
		case AX_LL_DEBUG:
			type = ax_u("DEBUG");
			ax_tcolor_fg(fp, AX_TCOLOR_BLUE);
			break;
		case AX_LL_INFO:
			type = ax_u("INFO");
			ax_tcolor_fg(fp, AX_TCOLOR_GREEN);
			break;
		case AX_LL_WARN:
			type = ax_u("WARN");
			ax_tcolor_fg(fp, AX_TCOLOR_YELLOW);
			break;
		case AX_LL_ERROR:
			type = ax_u("ERROR");
			ax_tcolor_fg(fp, AX_TCOLOR_RED);
			break;
		case AX_LL_FATAL:
			type = ax_u("FATAL");
			ax_tcolor_fg(fp, AX_TCOLOR_BRED);
			break;
		default:
			ax_usnprintf(level_buf, ax_nelems(level_buf), ax_u("%d"), level);
			type = level_buf;
			ax_tcolor_fg(fp, AX_TCOLOR_GREEN);
	}
	if (ax_fprintf(fp, ax_u("%-5" AX_PRIus), type) < 0) {
		ax_tcolor_reset(fp);
		goto out;
	}
	ax_tcolor_reset(fp);
	if (ax_fputs(ax_u("]"), fp) < 0)
		goto out;

	if (time_buf[0]) {
		if (ax_fputc(ax_u('['), fp) < 0)
			goto out;
		ax_tcolor_set(fp);
		ax_tcolor_fg(fp, AX_TCOLOR_GREEN);
		if (ax_fprintf(fp, ax_u("%" AX_PRIus), time_buf) < 0) {
			ax_tcolor_reset(fp);
			goto out;
		}
		ax_tcolor_reset(fp);
		if (ax_fputc(ax_u(']'), fp) < 0) {
			goto out;
		}
	}

	if (loc_buf[0]) {
		if (ax_fputc(ax_u('['), fp) < 0)
			goto out;
		ax_tcolor_set(fp);
		ax_tcolor_fg(fp, AX_TCOLOR_GREEN);
		if (ax_fprintf(fp, ax_u("%" AX_PRIus), loc_buf) < 0) {
			ax_tcolor_reset(fp);
			goto out;
		}
		ax_tcolor_reset(fp);
		if (ax_fputc(ax_u(']'), fp) < 0) {
			goto out;
		}
	}

	if (ax_fprintf(fp, ax_u(" %" AX_PRIus "\n"), text) < 0)
		goto out;

	if (fflush(fp))
		goto out;
	retval = 0;
out:
	ax_mutex_unlock(&s_lock);
	return retval;
}

int __ax_log2_print(const ax_location *loc, int level, const ax_uchar* fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	int retval = __ax_log2_vprint(loc, level, fmt, ap);
	va_end(ap);
	return retval;
}

int __ax_log2_vprint(const ax_location *loc, int level, const ax_uchar* fmt, va_list ap)
{
	int mode = ax_log_mode();
	switch(level)
	{
		default:
		case AX_LL_DEBUG:
			if (mode & AX_LM_NODEBUG)
				return 0;
			break;
		case AX_LL_INFO:
			if (mode & AX_LM_NOINFO)
				return 0;
			break;
		case AX_LL_WARN:
			if (mode & AX_LM_NOWARN)
				return 0;
			break;
		case AX_LL_ERROR:
			if (mode & AX_LM_NOERROR)
				return 0;
			break;
		case AX_LL_FATAL:
			if (mode & AX_LM_NOFATAL)
				return 0;
			break;
	}

	ax_uchar ms_buf[AX_LOG_MAX];
	if (ax_uvsnprintf(ms_buf, ax_nelems(ms_buf), fmt, ap) < 0)
		return -1;

	int ret = s_handler ? s_handler(loc, s_handler_arg, level, ms_buf)
		: log2_default_handler(loc, s_handler_arg, level, ms_buf);
	return ret;
}

