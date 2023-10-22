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

#ifndef UI_COMBOBOX_H
#define UI_COMBOBOX_H

#include "widget.h"

#ifndef UI_COMBOBOX_DEFINED
#define UI_COMBOBOX_DEFINED
typedef struct ui_combobox_st ui_combobox;
#endif

#define ax_baseof_ui_combobox ui_widget
ax_concrete_declare(2, ui_combobox);
ax_concrete_creator0(ui_combobox);

void ui_combobox_append(ui_combobox *c, const char *text);

void ui_combobox_insert(ui_combobox *c, int index, const char *text);

void ui_combobox_remove(ui_combobox *c, int index);

void ui_combobox_clear(ui_combobox *c);

int ui_combobox_num_items(const ui_combobox *c);

int ui_combobox_selected(const ui_combobox *c);

void ui_combobox_select(ui_combobox *c, int index);

void ui_combobox_on_selected(ui_combobox *c, void (*f)(ui_combobox *sender, void *senderData), void *data);

#endif

