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

#ifndef UI_TABLE_H
#define UI_TABLE_H

#include "ui/model.h"
#include "widget.h"

#ifndef UI_TABLE_DEFINED
#define UI_TABLE_DEFINED
typedef struct ui_table_st ui_table;
#endif

struct ui_table_header_st
{
	const char *name;
	int type;
};

#ifndef UI_TABLE_COLUMN_DEFINED
#define UI_TABLE_COLUMN_DEFINED
typedef struct ui_table_header_st ui_table_header;
#endif

#define ax_baseof_ui_table ui_widget
ax_concrete_declare(2, ui_table);
ax_concrete_creator(ui_table, const ui_table_header columns[]);

enum {
	UI_TABLE_SELECTION_NONE = 0,
	UI_TABLE_SELECTION_ONE,
	UI_TABLE_SELECTION_MANY,
};

size_t ui_table_num_items(const ui_table *t);

int *ui_table_selected(ui_table *t, size_t *count);

void ui_table_select(ui_table *t, const int *rows, size_t index);

void ui_table_on_selected(ui_table *t, void (*f)(ui_table *sender, void *arg), void *arg);

void ui_table_on_clicked(ui_table *t, void (*f)(ui_table *sender, int row, void *arg), void *arg);

void ui_table_on_double_clicked(ui_table *t, void (*f)(ui_table *sender, int row, void *arg), void *arg);

void ui_table_set_column_width(ui_table *t, int column, size_t width);

ui_model_r ui_table_get_model(ui_table *t);

void ui_table_set_selection_mode(ui_table *t, int mode);

int ui_table_selection_mode(const ui_table *t);

#endif

