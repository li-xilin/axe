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
#include "ui/slider.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define SLIDER(self) uiSlider(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_slider)
	void (*on_changed)(ui_slider *sender, void *arg);
	void (*on_released)(ui_slider *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_slider_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(SLIDER(self)));
	uiControlDestroy(uiControl(SLIDER(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_slider);
}

const ui_widget_trait ui_slider_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_slider, int min, int max)
{
        ui_widget *widget = NULL;
	uiSlider *slider = NULL;

	slider = uiNewSlider(min, max);
	if (!slider)
		goto fail;

        widget = malloc(sizeof(ui_slider));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(slider), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_slider slider_init = {
		.ui_widget = {
			.tr = &ui_slider_tr,
			.env.ctrl = slider,
		},
        };

        memcpy(widget, &slider_init, sizeof slider_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(slider));
        return NULL;
}

int ui_slider_value(const ui_slider *s)
{
	ui_slider_cr self = AX_R_INIT(ui_slider, s);
	return uiSliderValue(SLIDER(self));
}

void ui_slider_set_value(ui_slider *s, int value)
{
	ui_slider_r self = AX_R_INIT(ui_slider, s);
	uiSliderSetValue(SLIDER(self), value);
}

bool ui_slider_tooltip_enabled(const ui_slider *s)
{
	ui_slider_cr self = AX_R_INIT(ui_slider, s);
	return uiSliderHasToolTip(SLIDER(self));
}

void ui_slider_enable_tooltip(ui_slider *s, bool enable)
{
	ui_slider_r self = AX_R_INIT(ui_slider, s);
	uiSliderSetHasToolTip(SLIDER(self), enable);
}

static void OnChanged(uiSlider *sender, void *arg)
{
	ui_slider_r self = { control_data(uiControl(sender)) };
	self.ui_slider->on_changed(self.ui_slider, arg);
}

static void OnReleased(uiSlider *sender, void *arg)
{
	ui_slider_r self = { control_data(uiControl(sender)) };
	self.ui_slider->on_released(self.ui_slider, arg);
}

void ui_slider_on_changed(ui_slider *s, void (*f)(ui_slider *sender, void *senderData), void *arg)
{
	ui_slider_r self = AX_R_INIT(ui_slider, s);
	uiSliderOnChanged(SLIDER(self), f ? OnChanged : NULL, arg);
	self.ui_slider->on_changed = f;
}

void ui_slider_on_released(ui_slider *s, void (*f)(ui_slider *sender, void *senderData), void *arg)
{
	ui_slider_r self = AX_R_INIT(ui_slider, s);
	uiSliderOnReleased(SLIDER(self), f ? OnReleased : NULL, arg);
	self.ui_slider->on_released = f;
}

void ui_slider_set_range(ui_slider *s, int min, int max)
{
	ui_slider_r self = AX_R_INIT(ui_slider, s);
	uiSliderSetRange(SLIDER(self), min, max);
}

