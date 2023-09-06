/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to one person obtaining a copy
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

#include "ax/tcolor.h"
#include "ax/detect.h"

#include <stdio.h>

#ifdef AX_OS_WIN32
#include <windows.h>
static WORD _fg_colors[] = {
	[AX_TCOLOR_BLACK]   = 0,
	[AX_TCOLOR_RED]     = FOREGROUND_RED,
	[AX_TCOLOR_GREEN]   = FOREGROUND_GREEN,
	[AX_TCOLOR_YELLOW]  = FOREGROUND_RED | FOREGROUND_GREEN,
	[AX_TCOLOR_BLUE]    = FOREGROUND_BLUE,
	[AX_TCOLOR_MAGENTA] = FOREGROUND_RED | FOREGROUND_BLUE,
	[AX_TCOLOR_CYAN]    = FOREGROUND_GREEN | FOREGROUND_BLUE,
	[AX_TCOLOR_WHITE]   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE,

	[AX_TCOLOR_GREY]     = FOREGROUND_INTENSITY,
	[AX_TCOLOR_BRED]     = FOREGROUND_RED | FOREGROUND_INTENSITY,
	[AX_TCOLOR_BGREEN]   = FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	[AX_TCOLOR_BYELLOW]  = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY,
	[AX_TCOLOR_BBLUE]    = FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	[AX_TCOLOR_BMAGENTA] = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	[AX_TCOLOR_BCYAN]    = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
	[AX_TCOLOR_BWHITE]   = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY,
};

static WORD _bg_colors[] = {
	[AX_TCOLOR_BLACK]   = 0,
	[AX_TCOLOR_RED]     = BACKGROUND_RED,
	[AX_TCOLOR_GREEN]   = BACKGROUND_GREEN,
	[AX_TCOLOR_YELLOW]  = BACKGROUND_RED | BACKGROUND_GREEN,
	[AX_TCOLOR_BLUE]    = BACKGROUND_BLUE,
	[AX_TCOLOR_MAGENTA] = BACKGROUND_RED | BACKGROUND_BLUE,
	[AX_TCOLOR_CYAN]    = BACKGROUND_GREEN | BACKGROUND_BLUE,
	[AX_TCOLOR_WHITE]   = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE,

	[AX_TCOLOR_GREY]     = BACKGROUND_INTENSITY,
	[AX_TCOLOR_BRED]     = BACKGROUND_RED | BACKGROUND_INTENSITY,
	[AX_TCOLOR_BGREEN]   = BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	[AX_TCOLOR_BYELLOW]  = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY,
	[AX_TCOLOR_BBLUE]    = BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	[AX_TCOLOR_BMAGENTA] = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	[AX_TCOLOR_BCYAN]    = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
	[AX_TCOLOR_BWHITE]   = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY,
};

static WORD g_wErrAttr = 0;
static WORD g_wErrForeColor = 0;
static WORD g_wErrBackColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

static WORD g_wOutAttr = 0;
static WORD g_wOutForeColor = 0;
static WORD g_wOutBackColor = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;

#else

static const char *_fg_colors[] = {
	[AX_TCOLOR_BLACK]   = "\x1b[30m",
	[AX_TCOLOR_RED]     = "\x1b[31m",
	[AX_TCOLOR_GREEN]   = "\x1b[32m",
	[AX_TCOLOR_YELLOW]  = "\x1b[33m",
	[AX_TCOLOR_BLUE]    = "\x1b[34m",
	[AX_TCOLOR_MAGENTA] = "\x1b[35m",
	[AX_TCOLOR_CYAN]    = "\x1b[36m",
	[AX_TCOLOR_WHITE]   = "\x1b[37m",

	[AX_TCOLOR_GREY]     = "\x1b[90m",
	[AX_TCOLOR_BRED]     = "\x1b[91m",
	[AX_TCOLOR_BGREEN]   = "\x1b[92m",
	[AX_TCOLOR_BYELLOW]  = "\x1b[93m",
	[AX_TCOLOR_BBLUE]    = "\x1b[94m",
	[AX_TCOLOR_BMAGENTA] = "\x1b[95m",
	[AX_TCOLOR_BCYAN]    = "\x1b[96m",
	[AX_TCOLOR_BWHITE]   = "\x1b[97m",
};

static const char *_bg_colors[] = {
	[AX_TCOLOR_BLACK]   = "\x1b[40m",
	[AX_TCOLOR_RED]     = "\x1b[41m",
	[AX_TCOLOR_GREEN]   = "\x1b[42m",
	[AX_TCOLOR_YELLOW]  = "\x1b[43m",
	[AX_TCOLOR_BLUE]    = "\x1b[44m",
	[AX_TCOLOR_MAGENTA] = "\x1b[45m",
	[AX_TCOLOR_CYAN]    = "\x1b[46m",
	[AX_TCOLOR_WHITE]   = "\x1b[47m",

	[AX_TCOLOR_GREY]     = "\x1b[100m",
	[AX_TCOLOR_BRED]     = "\x1b[101m",
	[AX_TCOLOR_BGREEN]   = "\x1b[102m",
	[AX_TCOLOR_BYELLOW]  = "\x1b[103m",
	[AX_TCOLOR_BBLUE]    = "\x1b[104m",
	[AX_TCOLOR_BMAGENTA] = "\x1b[105m",
	[AX_TCOLOR_BCYAN]    = "\x1b[106m",
	[AX_TCOLOR_BWHITE]   = "\x1b[107m",
};
#endif

void ax_tcolor_set(FILE *file) {
#ifdef AX_OS_WIN32
	HANDLE handle;
	CONSOLE_SCREEN_BUFFER_INFO csbi;

	if (file == stderr) {
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
		if (GetFileType(handle) != FILE_TYPE_CHAR)
			return;
		GetConsoleScreenBufferInfo(handle, &csbi);
		g_wErrAttr = csbi.wAttributes;
		g_wErrForeColor = csbi.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		g_wErrBackColor = csbi.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	}
	else if (file == stdout) {
		handle = GetStdHandle(STD_ERROR_HANDLE);
		if (GetFileType(handle) != FILE_TYPE_CHAR)
			return;
		GetConsoleScreenBufferInfo(handle, &csbi);
		g_wOutAttr = csbi.wAttributes;
		g_wOutForeColor = csbi.wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
		g_wOutBackColor = csbi.wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	}
	else
		return;


#endif
}

void ax_tcolor_reset(FILE *file) {
#ifdef AX_OS_WIN32
	HANDLE hOutput;
	if (file == stderr) {
		hOutput = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hOutput, g_wErrAttr);
	}
	else if (file == stdout) {
		hOutput = GetStdHandle(STD_ERROR_HANDLE);
		SetConsoleTextAttribute(hOutput, g_wOutAttr);
	}
	else
		return;
#else
	if (file != stderr && file != stdout)
		return;

	fputs("\x1b[0m", file);
#endif
}

void ax_tcolor_bold(FILE *file) {
#ifdef AX_OS_WIN32
	(void)file;
#else
	if (file != stderr && file != stdout)
		return;

	fputs("\x1b[1m", file);
#endif
}

void ax_tcolor_fg(FILE *file, int color) {
	if (color < 0)
		return;
#ifdef AX_OS_WIN32
	HANDLE handle;
	if (file == stderr) {
		handle = GetStdHandle(STD_ERROR_HANDLE);
		g_wErrForeColor = _fg_colors[color];
		SetConsoleTextAttribute(handle, g_wErrForeColor | g_wErrBackColor);
	}
	else if (file == stdout) {
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
		g_wOutForeColor = _fg_colors[color];
		SetConsoleTextAttribute(handle, g_wOutForeColor | g_wOutBackColor);
	}
	else
		return;

#else
	if (file != stderr && file != stdout)
		return;

	fputs(_fg_colors[color], file);
#endif
}

void ax_tcolor_bg(FILE *file, int color) {
	if (color < 0)
		return;
#ifdef AX_OS_WIN32
	HANDLE handle;
	if (file == stderr) {
		handle = GetStdHandle(STD_ERROR_HANDLE);
		g_wErrBackColor = _bg_colors[color];
		SetConsoleTextAttribute(handle, g_wErrForeColor | g_wErrBackColor);
	}
	else if (file == stdout) {
		handle = GetStdHandle(STD_OUTPUT_HANDLE);
		g_wOutBackColor = _bg_colors[color];
		SetConsoleTextAttribute(handle, g_wOutForeColor | g_wOutBackColor);
	}
	else
		return;
#else
	if (file != stderr && file != stdout)
		return;

	fputs(_bg_colors[color], file);
#endif
}
