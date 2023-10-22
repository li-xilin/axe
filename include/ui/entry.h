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

#ifndef UI_ENTRY_H
#define UI_ENTRY_H

#include "widget.h"

#ifndef UI_ENTRY_DEFINED
#define UI_ENTRY_DEFINED
typedef struct ui_entry_st ui_entry;
#endif

enum {
	UI_ENTRY_NORMAL,
	UI_ENTRY_PASSWORD,
	UI_ENTRY_SEARCH,
	UI_ENTRY_SCROLLABLE,
	UI_ENTRY_VSCROLLABLE,
};

#define ax_baseof_ui_entry ui_widget
ax_concrete_declare(2, ui_entry);

ax_concrete_creator(ui_entry, int type, const char *text);

inline static ax_concrete_creator(ui_entry, int type)
{
	return ax_new(ui_entry, UI_ENTRY_NORMAL, NULL).ui_widget;
}

inline static ax_concrete_creator0(ui_entry)
{
	return ax_new(ui_entry, UI_ENTRY_NORMAL).ui_widget;
}

char *ui_entry_text(const ui_entry *b);

void ui_entry_set_text(ui_entry *b, const char *text);

void ui_entry_append(ui_entry *b, const char *text);

void ui_entry_on_changed(ui_entry *b, void (*proc)(ui_entry *sender, void *arg), void *arg);

bool ui_entry_readonly(const ui_entry *e);

void ui_entry_set_readonly(ui_entry *e, bool readonly);

bool ui_entry_readonly(const ui_entry *e);

#endif
