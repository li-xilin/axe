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

#ifndef UI_RADIOBOX_H
#define UI_RADIOBOX_H

#include "widget.h"

#ifndef UI_RADIOBOX_DEFINED
#define UI_RADIOBOX_DEFINED
typedef struct ui_radiobox_st ui_radiobox;
#endif

#define ax_baseof_ui_radiobox ui_widget
ax_concrete_declare(2, ui_radiobox);
ax_concrete_creator0(ui_radiobox);

void ui_radiobox_append(ui_radiobox *c, const char *text);

int ui_radiobox_selected(const ui_radiobox *c);

void ui_radiobox_select(ui_radiobox *c, int index);

void ui_radiobox_on_selected(ui_radiobox *c, void (*f)(ui_radiobox *sender, void *senderData), void *data);

#endif

