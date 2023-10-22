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

#include "ui/timepicker.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define TIMEPICKER(self) uiDateTimePicker(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_timepicker)
	void (*on_changed)(ui_timepicker *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_timepicker_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(TIMEPICKER(self)));
	uiControlDestroy(uiControl(TIMEPICKER(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_timepicker);
}

const ui_widget_trait ui_timepicker_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_timepicker, int type)
{
        ui_widget *widget = NULL;
	uiDateTimePicker *timepicker = NULL;

	switch (type)
	{
		case UI_TIMEPICKER_DATE:
			timepicker = uiNewDatePicker();
			break;
		case UI_TIMEPICKER_TIME:
			timepicker = uiNewTimePicker();
			break;
		case UI_TIMEPICKER_DATETIME:
			timepicker = uiNewDateTimePicker();
			break;
	}
	if (!timepicker)
		goto fail;

        if (!(widget = malloc(sizeof(ui_timepicker))))
                goto fail;

	if (control_attach(uiControl(timepicker), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_timepicker timepicker_init = {
		.ui_widget = {
			.tr = &ui_timepicker_tr,
			.env.ctrl = timepicker,
		},
        };

        memcpy(widget, &timepicker_init, sizeof timepicker_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(timepicker));
        return NULL;
}

static void OnChanged(uiDateTimePicker *sender, void *arg)
{
	ui_timepicker_r self = { control_data(uiControl(sender)) };
	self.ui_timepicker->on_changed(self.ui_timepicker, arg);
}

void ui_timepicker_on_changed(ui_timepicker *tp, void (*f)(ui_timepicker *sender, void *arg), void *arg)
{
	ui_timepicker_r self = AX_R_INIT(ui_timepicker, tp);
	uiDateTimePickerOnChanged(TIMEPICKER(self), f ? OnChanged : NULL, arg);
	self.ui_timepicker->on_changed = f;
}

void ui_timepicker_value(const ui_timepicker *tp, struct tm *value)
{
	ui_timepicker_cr self = AX_R_INIT(ui_timepicker, tp);
	uiDateTimePickerTime(TIMEPICKER(self), value);
}

void ui_timepicker_set_value(ui_timepicker *b, const struct tm *value)
{
	ui_timepicker_r self = AX_R_INIT(ui_timepicker, b);
	uiDateTimePickerSetTime(TIMEPICKER(self), value);
}

