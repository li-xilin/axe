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

#ifndef UI_GROUP_H
#define UI_GROUP_H

#include "widget.h"

#ifndef UI_GROUP_DEFINED
#define UI_GROUP_DEFINED
typedef struct ui_group_st ui_group;
#endif

#define ax_baseof_ui_group ui_widget
ax_concrete_declare(2, ui_group);
ax_concrete_creator(ui_group, const char *title);

char *ui_group_title(const ui_group *b);

void ui_group_set_title(ui_group *b, const char *text);

void ui_group_set_child(ui_group *g, ui_widget *c);

int ui_group_margined(const ui_group *g);

void ui_group_set_margined(ui_group *g, bool margined);

#endif

