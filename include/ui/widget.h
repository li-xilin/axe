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

#ifndef UI_WIDGET_H
#define UI_WIDGET_H
#include "ax/type/one.h"

#ifndef UI_WIDGET_DEFINED
#define UI_WIDGET_DEFINED
typedef struct ui_widget_st ui_widget;
#endif

#define ax_baseof_ui_widget ax_one

ax_abstract_data_begin(ui_widget)
	void *ctrl;
ax_end;

ax_abstract_code_begin(ui_widget)
	
ax_end;

ax_abstract_declare(1, ui_widget);

uintptr_t ui_widget_handle(const ui_widget *widget);

ui_widget *ui_widget_parent(const ui_widget *widget);

void ui_widget_set_parent(ui_widget *widget, ui_widget *parent);

int ui_widget_top_level(const ui_widget *widget);

bool ui_widget_visible(const ui_widget *widget);

void ui_widget_show(ui_widget *widget, bool show);

bool ui_widget_enabled(const ui_widget *widget);

void ui_widget_enable(ui_widget *widget, bool enable);

#endif

