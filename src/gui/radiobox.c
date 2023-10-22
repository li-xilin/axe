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

#include "ui/radiobox.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define RADIOBOX(self) uiRadioButtons(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_radiobox)
	void (*on_selected)(ui_radiobox *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_radiobox_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(RADIOBOX(self)));
	uiControlDestroy(uiControl(RADIOBOX(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_radiobox);
}

const ui_widget_trait ui_radiobox_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_radiobox, const char *text)
{
        ui_widget *widget = NULL;
	uiButton *radiobox = NULL;

	radiobox = uiNewButton(text);
	if (!radiobox)
		goto fail;

        widget = malloc(sizeof(ui_radiobox));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(radiobox), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_radiobox radiobox_init = {
		.ui_widget = {
			.tr = &ui_radiobox_tr,
			.env.ctrl = radiobox,
		},
        };

        memcpy(widget, &radiobox_init, sizeof radiobox_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(radiobox));
        return NULL;
}

void ui_radiobox_append(ui_radiobox *c, const char *text)
{
	ui_radiobox_r self = AX_R_INIT(ui_radiobox, c);
	uiRadioButtonsAppend(RADIOBOX(self), text);
}

int ui_radiobox_selected(const ui_radiobox *c)
{
	ui_radiobox_cr self = AX_R_INIT(ui_radiobox, c);
	return uiRadioButtonsSelected(RADIOBOX(self));
}

void ui_radiobox_select(ui_radiobox *c, int index)
{
	ui_radiobox_r self = AX_R_INIT(ui_radiobox, c);
	uiRadioButtonsSetSelected(RADIOBOX(self), index);
}

static void OnSelected(uiRadioButtons *sender, void *arg)
{
	ui_radiobox_r self = { control_data(uiControl(sender)) };
	self.ui_radiobox->on_selected(self.ui_radiobox, arg);
}

void ui_radiobox_on_selected(ui_radiobox *c, void (*f)(ui_radiobox *sender, void *senderData), void *arg)
{
	ui_radiobox_r self = AX_R_INIT(ui_radiobox, c);
	uiRadioButtonsOnSelected(RADIOBOX(self), f ? OnSelected : NULL, arg);
	self.ui_radiobox->on_selected = f;
}

