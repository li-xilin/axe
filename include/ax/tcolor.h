#ifndef AX_TCOLOR_H
#define AX_TCOLOR_H

#include <stdio.h>

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
