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

#ifndef AX_TCOLOR_H
#define AX_TCOLOR_H

#include "io.h"

enum {
	AX_TCOLOR_BLACK = 0,
	AX_TCOLOR_RED,
	AX_TCOLOR_GREEN,
	AX_TCOLOR_YELLOW,
	AX_TCOLOR_BLUE,
	AX_TCOLOR_MAGENTA,
	AX_TCOLOR_CYAN,
	AX_TCOLOR_WHITE,

	AX_TCOLOR_GREY,
	AX_TCOLOR_BRED,
	AX_TCOLOR_BGREEN,
	AX_TCOLOR_BYELLOW,
	AX_TCOLOR_BBLUE,
	AX_TCOLOR_BMAGENTA,
	AX_TCOLOR_BCYAN,
	AX_TCOLOR_BWHITE,
};

void ax_tcolor_set(FILE *file);
void ax_tcolor_reset(FILE *file);
void ax_tcolor_bold( FILE *file); /* Does not do anything on Windows */

void ax_tcolor_fg(FILE *file, int color);
void ax_tcolor_bg(FILE *file, int color);

#endif
