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

#ifndef UI_BOX_H
#define UI_BOX_H
#include "widget.h"

#ifndef UI_BOX_DEFINED
#define UI_BOX_DEFINED
typedef struct ui_box_st ui_box;
#endif

enum {
	UI_BOX_VERTICAL = 0,
	UI_BOX_HORIZONTAL,
};

#define ax_baseof_ui_box ui_widget
ax_concrete_declare(2, ui_box);

void ui_box_append(ui_box *b, ui_widget *child, bool stretchy);

int ui_box_count(const ui_box *b);

void ui_box_remove(ui_box *b, int index);

int ui_box_padded(ui_box *b);

void ui_box_set_padded(ui_box *b, int padded);

ax_concrete_creator(ui_box, int type);

inline static ax_concrete_creator0(ui_box)
{
	return ax_new(ui_box, UI_BOX_HORIZONTAL).ui_widget;
}

#endif

