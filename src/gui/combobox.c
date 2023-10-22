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

#include "ui/combobox.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define COMBOBOX(self) uiCombobox(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_combobox)
	void (*on_selected)(ui_combobox *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_combobox_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(COMBOBOX(self)));
	uiControlDestroy(uiControl(COMBOBOX(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_combobox);
}

const ui_widget_trait ui_combobox_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_combobox, const char *text)
{
        ui_widget *widget = NULL;
	uiButton *combobox = NULL;

	combobox = uiNewButton(text);
	if (!combobox)
		goto fail;

        widget = malloc(sizeof(ui_combobox));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(combobox), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_combobox combobox_init = {
		.ui_widget = {
			.tr = &ui_combobox_tr,
			.env.ctrl = combobox,
		},
        };

        memcpy(widget, &combobox_init, sizeof combobox_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(combobox));
        return NULL;
}

void ui_combobox_append(ui_combobox *c, const char *text)
{
	ui_combobox_r self = AX_R_INIT(ui_combobox, c);
	uiComboboxAppend(COMBOBOX(self), text);
}

void ui_combobox_insert(ui_combobox *c, int index, const char *text)
{
	ui_combobox_r self = AX_R_INIT(ui_combobox, c);
	uiComboboxInsertAt(COMBOBOX(self), index, text);
}

void ui_combobox_remove(ui_combobox *c, int index)
{
	ui_combobox_r self = AX_R_INIT(ui_combobox, c);
	uiComboboxDelete(COMBOBOX(self), index);
}

void ui_combobox_clear(ui_combobox *c)
{
	ui_combobox_r self = AX_R_INIT(ui_combobox, c);
	uiComboboxClear(COMBOBOX(self));
}

int ui_combobox_num_items(const ui_combobox *c)
{
	ui_combobox_cr self = AX_R_INIT(ui_combobox, c);
	return uiComboboxNumItems(COMBOBOX(self));
}

int ui_combobox_selected(const ui_combobox *c)
{
	ui_combobox_cr self = AX_R_INIT(ui_combobox, c);
	return uiComboboxSelected(COMBOBOX(self));
}

void ui_combobox_select(ui_combobox *c, int index)
{
	ui_combobox_r self = AX_R_INIT(ui_combobox, c);
	uiComboboxSetSelected(COMBOBOX(self), index);
}

static void OnSelected(uiCombobox *sender, void *arg)
{
	ui_combobox_r self = { control_data(uiControl(sender)) };
	self.ui_combobox->on_selected(self.ui_combobox, arg);
}

void ui_combobox_on_selected(ui_combobox *c, void (*f)(ui_combobox *sender, void *senderData), void *arg)
{
	ui_combobox_r self = AX_R_INIT(ui_combobox, c);
	uiComboboxOnSelected(COMBOBOX(self), f ? OnSelected : NULL, arg);
	self.ui_combobox->on_selected = f;
}

