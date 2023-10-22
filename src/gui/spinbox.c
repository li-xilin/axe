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

#include "ui/spinbox.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define SPINBOX(self) uiSpinbox(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_spinbox)
	void (*on_changed)(ui_spinbox *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_spinbox_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(SPINBOX(self)));
	uiControlDestroy(uiControl(SPINBOX(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_spinbox);
}

const ui_widget_trait ui_spinbox_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_spinbox, int min, int max)
{
        ui_widget *widget = NULL;
	uiSpinbox *spinbox = NULL;

	spinbox = uiNewSpinbox(min, max);
	if (!spinbox)
		goto fail;

        if (!(widget = malloc(sizeof(ui_spinbox))))
                goto fail;

	if (control_attach(uiControl(spinbox), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_spinbox spinbox_init = {
		.ui_widget = {
			.tr = &ui_spinbox_tr,
			.env.ctrl = spinbox,
		},
        };

        memcpy(widget, &spinbox_init, sizeof spinbox_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(spinbox));
        return NULL;
}

int ui_spinbox_text(const ui_spinbox *b)
{
	ui_spinbox_cr self = AX_R_INIT(ui_spinbox, b);
	return uiSpinboxValue(SPINBOX(self));
}

void ui_spinbox_set_text(ui_spinbox *b, int value)
{
	ui_spinbox_r self = AX_R_INIT(ui_spinbox, b);
	uiSpinboxSetValue(SPINBOX(self), value);
}

static void OnChanged(uiSpinbox *sender, void *arg)
{
	ui_spinbox_r self = { control_data(uiControl(sender)) };
	self.ui_spinbox->on_changed(self.ui_spinbox, arg);
}

void ui_spinbox_on_changed(ui_spinbox *b, void (*f)(ui_spinbox *sender, void *arg), void *arg)
{
	ui_spinbox_r self = AX_R_INIT(ui_spinbox, b);
	uiSpinboxOnChanged(SPINBOX(self), f ? OnChanged : NULL, arg);
	self.ui_spinbox->on_changed = f;
}

