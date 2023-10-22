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

#include "ui/label.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define LABEL(self) uiLabel(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_label)
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_label_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(LABEL(self)));
	uiControlDestroy(uiControl(LABEL(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_label);
}

const ui_widget_trait ui_label_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_label, const char *text)
{
        ui_widget *widget = NULL;
	uiLabel *label = NULL;

	label = uiNewLabel(text);
	if (!label)
		goto fail;

        widget = malloc(sizeof(ui_label));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(label), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_label label_init = {
		.ui_widget = {
			.tr = &ui_label_tr,
			.env.ctrl = label,
		},
        };

        memcpy(widget, &label_init, sizeof label_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(label));
        return NULL;
}

char *ui_label_text(const ui_label *label)
{
	ui_label_cr self = AX_R_INIT(ui_label, label);
	return uiLabelText(LABEL(self));
}

void ui_label_settext(ui_label *label, const char *text)
{
	ui_label_r self = AX_R_INIT(ui_label, label);
	uiLabelSetText(LABEL(self), text);
}

