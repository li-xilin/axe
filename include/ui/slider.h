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

#ifndef UI_SLIDER_H
#define UI_SLIDER_H

#include "widget.h"

#ifndef UI_SLIDER_DEFINED
#define UI_SLIDER_DEFINED
typedef struct ui_slider_st ui_slider;
#endif

#define ax_baseof_ui_slider ui_widget

ax_concrete_declare(2, ui_slider);

ax_concrete_creator(ui_slider, const char *text);

int ui_slider_value(const ui_slider *s);

void ui_slider_set_value(ui_slider *s, int value);

bool ui_slider_tooltip_enabled(const ui_slider *s);

void ui_slider_enable_tooltip(ui_slider *s, bool enable);

void ui_slider_on_changed(ui_slider *s, void (*f)(ui_slider *sender, void *senderData), void *data);

void ui_slider_on_released(ui_slider *s, void (*f)(ui_slider *sender, void *senderData), void *data);

void ui_slider_set_range(ui_slider *s, int min, int max);

#endif

