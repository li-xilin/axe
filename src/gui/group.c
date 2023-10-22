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

#include "ui/group.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define GROUP(self) uiGroup(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_group)
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_group_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(GROUP(self)));
	uiControlDestroy(uiControl(GROUP(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_group);
}

const ui_widget_trait ui_group_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_group, const char *title)
{
        ui_widget *widget = NULL;
	uiGroup *group = NULL;

	group = uiNewGroup(title);
	if (!group)
		goto fail;

        widget = malloc(sizeof(ui_group));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(group), ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_group group_init = {
		.ui_widget = {
			.tr = &ui_group_tr,
			.env.ctrl = group,
		},
        };

        memcpy(widget, &group_init, sizeof group_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(group));
        return NULL;
}

char *ui_group_title(const ui_group *b)
{
	ui_group_cr self = AX_R_INIT(ui_group, b);
	return uiGroupTitle(GROUP(self));
}

void ui_group_set_title(ui_group *b, const char *text)
{
	ui_group_r self = AX_R_INIT(ui_group, b);
	uiGroupSetTitle(GROUP(self), text);
}

void ui_group_set_child(ui_group *g, ui_widget *c)
{
	ui_group_r self = AX_R_INIT(ui_group, g);
	uiGroupSetChild(GROUP(self), ax_class_data(c).ctrl);
}

int ui_group_margined(const ui_group *g)
{
	ui_group_cr self = AX_R_INIT(ui_group, g);
	return uiGroupMargined(GROUP(self));
}

void ui_group_set_margined(ui_group *g, bool margined)
{
	ui_group_r self = AX_R_INIT(ui_group, g);
	uiGroupSetMargined(GROUP(self), margined);
}

