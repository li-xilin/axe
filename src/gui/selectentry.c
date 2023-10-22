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
#include "ui/selectentry.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define SELECTENTRY(self) uiEditableCombobox(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_selectentry)
	void (*on_changed)(ui_selectentry *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_selectentry_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(SELECTENTRY(self)));
	uiControlDestroy(uiControl(SELECTENTRY(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_selectentry);
}

const ui_widget_trait ui_selectentry_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator0(ui_selectentry)
{
        ui_widget *widget = NULL;
	uiEditableCombobox *control = NULL;

	control = uiNewEditableCombobox();
	if (!control)
		goto fail;

        widget = malloc(sizeof(ui_selectentry));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(control), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_selectentry selectentry_init = {
		.ui_widget = {
			.tr = &ui_selectentry_tr,
			.env.ctrl = control,
		},
        };

        memcpy(widget, &selectentry_init, sizeof selectentry_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(control));
        return NULL;
}

char *ui_selectentry_text(const ui_selectentry *b)
{
	ui_selectentry_cr self = AX_R_INIT(ui_selectentry, b);
	return uiEditableComboboxText(SELECTENTRY(self));
}

void ui_selectentry_set_text(ui_selectentry *b, const char *text)
{
	ui_selectentry_r self = AX_R_INIT(ui_selectentry, b);
	uiEditableComboboxSetText(SELECTENTRY(self), text);
}

void ui_selectentry_append(ui_selectentry *c, const char *text)
{
	ui_selectentry_r self = AX_R_INIT(ui_selectentry, c);
	uiEditableComboboxAppend(SELECTENTRY(self), text);
}

static void OnChanged(uiEditableCombobox *sender, void *arg)
{
	ui_selectentry_r self = { control_data(uiControl(sender)) };
	self.ui_selectentry->on_changed(self.ui_selectentry, arg);
}

void ui_selectentry_on_clicked(ui_selectentry *b, void (*f)(ui_selectentry *sender, void *arg), void *arg)
{
	ui_selectentry_r self = AX_R_INIT(ui_selectentry, b);
	uiEditableComboboxOnChanged(SELECTENTRY(self), f ? OnChanged : NULL, arg);
	self.ui_selectentry->on_changed = f;
}

