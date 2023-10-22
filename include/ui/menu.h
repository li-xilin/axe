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

#ifndef UI_MENU_H
#define UI_MENU_H

#include "ax/type/one.h"

#ifndef UI_MENU_DEFINED
#define UI_MENU_DEFINED
typedef struct ui_menu_st ui_menu;
#endif

#ifndef UI_MENU_ITEM_DEFINED
#define UI_MENU_ITEM_DEFINED
typedef struct ui_menu_item_st ui_menu_item;
#endif

#ifndef UI_WINDOW_DEFINED
#define UI_WINDOW_DEFINED
typedef struct ui_window_st ui_window;
#endif

#define ax_baseof_ui_menu ax_one
ax_concrete_declare(1, ui_menu);
ax_concrete_creator(ui_menu, const char *text);

enum {
	UI_MENU_ITEM_TEXT = 0,
	UI_MENU_ITEM_CHECKABLE,
	UI_MENU_ITEM_QUIT,
	UI_MENU_ITEM_PREFERENCE,
	UI_MENU_ITEM_SEPARATOR,
};

ui_menu_item *ui_menu_append_item(ui_menu *m, const char *text);
ui_menu_item *ui_menu_append_checkitem(ui_menu *m, const char *text);
ui_menu_item *ui_menu_append_preferences(ui_menu *m);
ui_menu_item *ui_menu_append_about(ui_menu *m);
void ui_menu_append_quit(ui_menu *m);
void ui_menu_append_separator(ui_menu *m);

int ui_menu_item_on_clicked(ui_menu_item *mi, void (*f)(ui_menu_item *mi, ui_window *w, void *arg), void *arg);

void ui_menu_item_set_enable(ui_menu_item *mi, bool enable);
void ui_menu_item_set_checked(ui_menu_item *mi, bool checked);
bool ui_menu_item_checked(const ui_menu_item *mi);

#endif

