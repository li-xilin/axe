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

#include "ui/checkbox.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define CHECKBOX(self) uiCheckbox(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_checkbox)
	void (*on_toggled)(ui_checkbox *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_checkbox_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(CHECKBOX(self)));
	uiControlDestroy(uiControl(CHECKBOX(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_checkbox);
}

const ui_widget_trait ui_checkbox_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_checkbox, const char *text)
{
        ui_widget *widget = NULL;
	uiCheckbox *checkbox = NULL;

	checkbox = uiNewCheckbox(text);
	if (!checkbox)
		goto fail;

        widget = malloc(sizeof(ui_checkbox));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(checkbox), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_checkbox checkbox_init = {
		.ui_widget = {
			.tr = &ui_checkbox_tr,
			.env.ctrl = checkbox,
		},
        };

        memcpy(widget, &checkbox_init, sizeof checkbox_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(checkbox));
        return NULL;
}

char *ui_checkbox_text(const ui_checkbox *c)
{
	ui_checkbox_cr self = AX_R_INIT(ui_checkbox, c);
	return uiCheckboxText(CHECKBOX(self));
}

void ui_checkbox_set_text(ui_checkbox *c, const char *text)
{
	ui_checkbox_r self = AX_R_INIT(ui_checkbox, c);
	uiCheckboxSetText(CHECKBOX(self), text);
}

int ui_checkbox_checked(const ui_checkbox *c)
{
	ui_checkbox_cr self = AX_R_INIT(ui_checkbox, c);
	return uiCheckboxChecked(CHECKBOX(self));
}

void ui_checkbox_check(ui_checkbox *c, bool checked)
{
	ui_checkbox_r self = AX_R_INIT(ui_checkbox, c);
	uiCheckboxSetChecked(CHECKBOX(self), checked);
}

static void OnToggled(uiCheckbox *sender, void *arg)
{
	ui_checkbox_r self = { control_data(uiControl(sender)) };
	self.ui_checkbox->on_toggled(self.ui_checkbox, arg);
}

void ui_checkbox_on_toggled(ui_checkbox *c, void (*f)(ui_checkbox *sender, void *arg), void *arg)
{
	ui_checkbox_r self = AX_R_INIT(ui_checkbox, c);
	uiCheckboxOnToggled(CHECKBOX(self), f ? OnToggled : NULL, arg);
	self.ui_checkbox->on_toggled = f;
}

