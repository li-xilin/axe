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

#ifndef UI_SELECTENTRY_H
#define UI_SELECTENTRY_H

#include "widget.h"

#ifndef UI_SELECTENTRY_DEFINED
#define UI_SELECTENTRY_DEFINED
typedef struct ui_selectentry_st ui_selectentry;
#endif

#define ax_baseof_ui_selectentry ui_widget
ax_concrete_declare(2, ui_selectentry);
ax_concrete_creator0(ui_selectentry);

char *ui_selectentry_text(const ui_selectentry *b);

void ui_selectentry_set_text(ui_selectentry *b, const char *text);

void ui_selectentry_append(ui_selectentry *c, const char *text);

void ui_selectentry_onchanged(ui_selectentry *c, void (*f)(ui_selectentry *sender, void *senderData), void *data);

#endif
