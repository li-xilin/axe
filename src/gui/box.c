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

#include "ui/box.h"
#include "ui/types.h"

#include <ui.h>

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#define CONTROL(self) uiControl(ax_class_data(self.ui_widget).ctrl)
#define BOX(self) uiBox(CONTROL(self))

ax_concrete_begin(ui_box)
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_box_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(BOX(self)));
	uiControlDestroy(uiControl(BOX(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_box);
}

const ui_widget_trait ui_box_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_box, int type)
{

        ui_widget *widget = NULL;
	uiBox *box = NULL;

	switch (type) {
		case UI_BOX_VERTICAL:
			box = uiNewVerticalBox();
			break;
		case UI_BOX_HORIZONTAL:
			box = uiNewHorizontalBox();
			break;
		default:
			errno = EINVAL;
			goto fail;
	}
	if (!box)
		goto fail;

        widget = malloc(sizeof(ui_box));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(box), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_box label_init = {
		.ui_widget = {
			.tr = &ui_box_tr,
			.env.ctrl = box,
		},
        };

        memcpy(widget, &label_init, sizeof label_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(box));
        return NULL;
}

void ui_box_append(ui_box *b, ui_widget *child, bool stretchy)
{
	ui_box_r self = AX_R_INIT(ui_box, b);
	uiBoxAppend(BOX(self), ax_class_data(child).ctrl, stretchy);
}

int ui_box_count(const ui_box *b)
{
	ui_box_cr self = AX_R_INIT(ui_box, b);
	return uiBoxNumChildren(BOX(self));
}

void ui_box_remove(ui_box *b, int index)
{
	ui_box_r self = AX_R_INIT(ui_box, b);
	uiBoxDelete(BOX(self), index);
}

int ui_box_padded(ui_box *b)
{
	ui_box_cr self = AX_R_INIT(ui_box, b);
	return uiBoxPadded(BOX(self));
}

void ui_box_set_padded(ui_box *b, int padded)
{
	ui_box_r self = AX_R_INIT(ui_box, b);
	uiBoxSetPadded(BOX(self), padded);
}

