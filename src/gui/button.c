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

#include "ui/button.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define BUTTON(self) uiButton(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_button)
	void (*on_click)(ui_button *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_button_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(BUTTON(self)));
	uiControlDestroy(uiControl(BUTTON(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_button);
}

const ui_widget_trait ui_button_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_button, const char *text)
{
        ui_widget *widget = NULL;
	uiButton *button = NULL;

	button = uiNewButton(text);
	if (!button)
		goto fail;

        widget = malloc(sizeof(ui_button));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(button), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_button button_init = {
		.ui_widget = {
			.tr = &ui_button_tr,
			.env.ctrl = button,
		},
        };

        memcpy(widget, &button_init, sizeof button_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(button));
        return NULL;
}

char *ui_button_text(const ui_button *b)
{
	ui_button_cr self = AX_R_INIT(ui_button, b);
	return uiButtonText(BUTTON(self));
}

void ui_button_set_text(ui_button *b, const char *text)
{
	ui_button_r self = AX_R_INIT(ui_button, b);
	uiButtonSetText(BUTTON(self), text);
}

static void OnClick(uiButton *sender, void *arg)
{
	ui_button_r self = { control_data(uiControl(sender)) };
	self.ui_button->on_click(self.ui_button, arg);
}

void ui_button_on_clicked(ui_button *b, void (*f)(ui_button *sender, void *arg), void *arg)
{
	ui_button_r self = AX_R_INIT(ui_button, b);
	uiButtonOnClicked(BUTTON(self), f ? OnClick : NULL, arg);
	self.ui_button->on_click = f;
}

