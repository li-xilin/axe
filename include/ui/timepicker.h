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

#ifndef UI_TIMEPICKER_H
#define UI_TIMEPICKER_H

#include "widget.h"
#include <time.h>

#ifndef UI_TIMEPICKER_DEFINED
#define UI_TIMEPICKER_DEFINED
typedef struct ui_timepicker_st ui_timepicker;
#endif

enum {
	UI_TIMEPICKER_DATE,
	UI_TIMEPICKER_TIME,
	UI_TIMEPICKER_DATETIME,
};

#define ax_baseof_ui_timepicker ui_widget
ax_concrete_declare(2, ui_timepicker);

void ui_timepicker_value(const ui_timepicker *tp, struct tm *value);

void ui_timepicker_set_value(ui_timepicker *tp, const struct tm *value);

void ui_timepicker_on_changed(ui_timepicker *tp, void (*proc)(ui_timepicker *sender, void *arg), void *arg);

ax_concrete_creator(ui_timepicker, int type);

#endif
