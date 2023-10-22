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

#include "ui/form.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define FORM(self) uiForm(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_form)
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_form_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(FORM(self)));
	uiControlDestroy(uiControl(FORM(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_form);
}

const ui_widget_trait ui_form_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator0(ui_form)
{
        ui_widget *widget = NULL;
	uiForm *form = NULL;

	form = uiNewForm();
	if (!form)
		goto fail;

        widget = malloc(sizeof(ui_form));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(form), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_form form_init = {
		.ui_widget = {
			.tr = &ui_form_tr,
			.env.ctrl = form,
		},
        };

        memcpy(widget, &form_init, sizeof form_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(form));
        return NULL;
}

void ui_form_append(ui_form *f, const char *label, ui_widget *c, bool stretchy)
{
	ui_form_r self = AX_R_INIT(ui_form, f);
	uiFormAppend(FORM(self), label, ax_class_data(c).ctrl, stretchy);
}

int ui_form_num_widgets(const ui_form *f)
{
	ui_form_cr self = AX_R_INIT(ui_form, f);
	return uiFormNumChildren(FORM(self));
}

void ui_from_remove(ui_form *f, int index)
{
	ui_form_r self = AX_R_INIT(ui_form, f);
	uiFormDelete(FORM(self), index);
}

bool ui_form_padded(const ui_form *f)
{
	ui_form_cr self = AX_R_INIT(ui_form, f);
	return uiFormPadded(FORM(self));
}

void ui_form_set_padded(ui_form *f, bool padded)
{
	ui_form_r self = AX_R_INIT(ui_form, f);
	uiFormSetPadded(FORM(self), padded);
}

