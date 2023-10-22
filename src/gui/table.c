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

#include "control.h"

#include "ui/table.h"
#include "ui/model.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define TABLE(self) uiTable(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_table)
	ui_model_r model;
	void (*on_clicked)(ui_table *sender, int row, void *arg);
	void (*on_double_clicked)(ui_table *sender, int row, void *arg);
	void (*on_selected)(ui_table *sender, void *arg);
ax_end;

extern void __ui_model_free(ui_model *m);

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_table_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(TABLE(self)));
	uiControlDestroy(uiControl(TABLE(self)));
	__ui_model_free(self.ui_table->model.ui_model);
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_table);
}

const ui_widget_trait ui_table_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

static void create_headers(uiTable *t, const ui_table_header *columns, size_t count)
{
	int off = 0;
	for (int i = 0; i < count; i++, off = i * 4) {
		switch (columns[i].type) {
			case UI_MODEL_TEXT:
				uiTableAppendTextColumn(t, columns[i].name, off, off + 1, NULL);
				break;
			case UI_MODEL_CHECKBOX:
				uiTableAppendCheckboxColumn(t, columns[i].name, off, off + 1);
				break;
			case UI_MODEL_BUTTON:
				uiTableAppendButtonColumn(t, columns[i].name, off, off + 1);
				break;
			case UI_MODEL_IMAGE:
				uiTableAppendImageColumn(t, columns[i].name, off);
				break;
			case UI_MODEL_PROGRESS:
				uiTableAppendProgressBarColumn(t, columns[i].name, off);
				break;
			case UI_MODEL_CHECKBOX_TEXT:
				uiTableAppendCheckboxTextColumn(t, columns[i].name, off, off + 1, off + 2, off + 3, NULL);
				break;
			case UI_MODEL_IMAGE_TEXT:
				uiTableAppendImageTextColumn(t, columns[i].name, off, off + 1, off + 2, NULL);
				break;
		}
	}
}

ax_concrete_creator(ui_table, const ui_table_header *headers)
{
        ui_widget *widget = NULL;
	uiTable *table = NULL;
	uiTableParams params = { NULL };
	ui_model_r model = AX_R_NULL;

	size_t count;
	for (count = 0; headers[count].name; count++);

	int *types = malloc(sizeof(int) * count);
	for (int i = 0; i < count; i++) {
		types[i] = headers[i].type;
	}
	model = ax_new(ui_model, types, count);
	free(types);
	if (!model.ax_one)
		goto fail;

	params.Model = ui_model_raw_model(model.ui_model);
	params.RowBackgroundColorModelColumn = -1;

	table = uiNewTable(&params);
	if (!table)
		goto fail;

	create_headers(table, headers, count);

        widget = malloc(sizeof(ui_table));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(table), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_table table_init = {
		.ui_widget = {
			.tr = &ui_table_tr,
			.env.ctrl = table,
		},
		.model = model,
        };

        memcpy(widget, &table_init, sizeof table_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(table));
        return NULL;
}

void ui_table_set_column_width(ui_table *t, int column, size_t width)
{
	ui_table_r self = AX_R_INIT(ui_table, t);
	uiTableColumnSetWidth(TABLE(self), column, width);
}

static void OnClicked(uiTable *sender, int row, void *arg)
{
	ui_table_r self = { control_data(uiControl(sender)) };
	self.ui_table->on_clicked(self.ui_table, row, arg);
}

void ui_table_on_clicked(ui_table *t, void (*f)(ui_table *sender, int row, void *arg), void *arg)
{
	ui_table_r self = AX_R_INIT(ui_table, t);
	uiTableOnRowClicked(TABLE(self), f ? OnClicked : NULL, arg);
	self.ui_table->on_clicked = f;
}

static void OnDoubleClicked(uiTable *sender, int row, void *arg)
{
	ui_table_r self = { control_data(uiControl(sender)) };
	self.ui_table->on_double_clicked(self.ui_table, row, arg);
}

void ui_table_on_double_clicked(ui_table *t, void (*f)(ui_table *sender, int row, void *arg), void *arg)
{
	ui_table_r self = AX_R_INIT(ui_table, t);
	uiTableOnRowDoubleClicked(TABLE(self), f ? OnDoubleClicked : NULL, arg);
	self.ui_table->on_double_clicked = f;
}

static void OnSelected(uiTable *sender, void *arg)
{
	ui_table_r self = { control_data(uiControl(sender)) };
	self.ui_table->on_selected(self.ui_table, arg);
}

void ui_table_on_selected(ui_table *t, void (*f)(ui_table *sender, void *arg), void *arg)
{
	ui_table_r self = AX_R_INIT(ui_table, t);
	uiTableOnSelectionChanged(TABLE(self), f ? OnSelected : NULL, arg);
	self.ui_table->on_selected = f;
}

ui_model_r ui_table_get_model(ui_table *t)
{
	return t->model;
}

size_t ui_table_num_items(const ui_table *t)
{
	return ax_box_size(t->model.ax_box);
}

void ui_table_select(ui_table *t, const int *rows, size_t count)
{
	ui_table_r self = AX_R_INIT(ui_table, t);
	uiTableSelection sel;
	sel.NumRows = count;
	sel.Rows = (int *)rows;
	uiTableSetSelection(TABLE(self), &sel);
}

int *ui_table_selected(ui_table *t, size_t *count)
{
	ui_table_r self = AX_R_INIT(ui_table, t);
	uiTableSelection *sel = uiTableGetSelection(TABLE(self));
	if (!sel)
		return NULL;
	*count = sel->NumRows;
	if (sel->NumRows == 0) {
		return NULL;
	}
	int *rows = malloc(sizeof(int) * sel->NumRows);
	if (!rows) {
		uiFreeTableSelection(sel);
		return NULL;
	}
	for (int i = 0; i < sel->NumRows; i++) {
		rows[i] = sel->Rows[i];
	}
	uiFreeTableSelection(sel);
	return rows;
}

void ui_table_set_selection_mode(ui_table *t, int mode)
{
	ui_table_r self = AX_R_INIT(ui_table, t);
	uiTableSelectionMode uimode;
	switch (mode) {
		case UI_TABLE_SELECTION_NONE:
			uimode = uiTableSelectionModeNone;
			break;
		case UI_TABLE_SELECTION_ONE:
			uimode = uiTableSelectionModeOne;
			break;
		case UI_TABLE_SELECTION_MANY:
			uimode = uiTableSelectionModeZeroOrMany;
			break;
		default:
			ax_assert(false, "invalid selection mode");
			return;
	}
	uiTableSetSelectionMode(TABLE(self), uimode);
}

int ui_table_selection_mode(const ui_table *t)
{
	ui_table_cr self = AX_R_INIT(ui_table, t);
	uiTableSelectionMode uimode = uiTableGetSelectionMode(TABLE(self));
	switch (uimode) {
		case uiTableSelectionModeNone:
			return UI_TABLE_SELECTION_NONE;
		case uiTableSelectionModeOne:
			return UI_TABLE_SELECTION_ONE;
		case uiTableSelectionModeZeroOrMany:
			return UI_TABLE_SELECTION_MANY;
		default:
			ax_assert(false, "invalid selection mode returned");
			return -1;
	}
}
