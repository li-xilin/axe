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

#include "ax/def.h"
#include "ui/colorpicker.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define COLORPICKER(self) uiColorButton(ax_class_data(self.ui_widget).ctrl)

void ui_colorpicker_set_value(ui_colorpicker *b, const ui_rgba *value);

ax_concrete_begin(ui_colorpicker)
	void (*on_changed)(ui_colorpicker *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_colorpicker_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(COLORPICKER(self)));
	uiControlDestroy(uiControl(COLORPICKER(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_colorpicker);
}

const ui_widget_trait ui_colorpicker_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_colorpicker, const ui_rgba *def_color)
{
        ui_widget *widget = NULL;
	uiColorButton *colorpicker = NULL;

	if (!(colorpicker = uiNewColorButton()))
		goto fail;

        if (!(widget = malloc(sizeof(ui_colorpicker))))
                goto fail;

	if (control_attach(uiControl(colorpicker), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_colorpicker colorpicker_init = {
		.ui_widget = {
			.tr = &ui_colorpicker_tr,
			.env.ctrl = colorpicker,
		},
        };

        memcpy(widget, &colorpicker_init, sizeof colorpicker_init);

	ui_colorpicker_set_value((ui_colorpicker *)widget, def_color);

        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(colorpicker));
        return NULL;
}

static void OnChanged(uiColorButton *sender, void *arg)
{
	ui_colorpicker_r self = { control_data(uiControl(sender)) };
	self.ui_colorpicker->on_changed(self.ui_colorpicker, arg);
}

void ui_colorpicker_on_changed(ui_colorpicker *tp, void (*f)(ui_colorpicker *sender, void *arg), void *arg)
{
	ui_colorpicker_r self = AX_R_INIT(ui_colorpicker, tp);
	uiColorButtonOnChanged(COLORPICKER(self), f ? OnChanged : NULL, arg);
	self.ui_colorpicker->on_changed = f;
}

void ui_colorpicker_value(const ui_colorpicker *tp, ui_rgba *value)
{
	ui_colorpicker_cr self = AX_R_INIT(ui_colorpicker, tp);
	double r, g, b, a;
	uiColorButtonColor(COLORPICKER(self), &r, &g, &b, &a);
	*value = *(ui_rgba []) { { 0xFF * r, 0xFF * g, 0xFF * b, 0xFF * a } };
}

void ui_colorpicker_set_value(ui_colorpicker *b, const ui_rgba *value)
{
	ui_colorpicker_r self = AX_R_INIT(ui_colorpicker, b);
	double tunnel[4] = { value->r / 255.0, value->g / 255.0, value->b / 255.0, value->a / 255.0 };
	uiColorButtonSetColor(COLORPICKER(self), tunnel[0], tunnel[1], tunnel[2], tunnel[3]);
}

