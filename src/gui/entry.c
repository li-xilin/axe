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

#include "ui/entry.h"
#include "ui/types.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#define ENTRY(self) uiEntry(ax_class_data(self.ui_widget).ctrl)
#define MENTRY(self) uiMultilineEntry(ax_class_data(self.ui_widget).ctrl)

ax_concrete_begin(ui_entry)
	bool mulitline;
	void (*on_changed)(ui_entry *sender, void *arg);
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_entry_r self = AX_R_INIT(ax_one, one);
	control_detach(uiControl(ENTRY(self)));
	uiControlDestroy(uiControl(ENTRY(self)));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_entry);
}

const ui_widget_trait ui_entry_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_entry, int type, const char *text)
{
        ui_widget *widget = NULL;
	uiEntry *entry = NULL;
	uiMultilineEntry *multiline_entry = NULL;
	bool multiline = false;

	switch (type) {
		case UI_ENTRY_NORMAL:
			entry = uiNewEntry();
			break;
		case UI_ENTRY_PASSWORD:
			entry = uiNewPasswordEntry();
			break;
		case UI_ENTRY_SEARCH:
			entry = uiNewSearchEntry();
			break;
		case UI_ENTRY_SCROLLABLE:
			multiline_entry = uiNewNonWrappingMultilineEntry();
			multiline = true;
			break;
		case UI_ENTRY_VSCROLLABLE:
			multiline_entry = uiNewMultilineEntry();
			multiline = true;
			break;
		default:
			errno = EINVAL;
			goto fail;
	}
	if (!entry && !multiline_entry)
		goto fail;

        if (!(widget = malloc(sizeof(ui_entry))))
                goto fail;

	if (text) {
		if (multiline)
			uiMultilineEntrySetText(multiline_entry, text);
		else
			uiEntrySetText(entry, text);
	}

	if (control_attach(multiline ? uiControl(multiline_entry) : uiControl(entry),
				ax_r(ui_widget, widget).ax_one))
		goto fail;

        ui_entry entry_init = {
		.ui_widget = {
			.tr = &ui_entry_tr,
			.env.ctrl = multiline
				? uiControl(multiline_entry)
				: uiControl(entry),
		},
		.mulitline = multiline,
        };

        memcpy(widget, &entry_init, sizeof entry_init);
        return widget;
fail:
        free(widget);
	if (entry)
		uiFreeControl(uiControl(entry));
	if (multiline_entry)
		uiFreeControl(uiControl(multiline_entry));
        return NULL;
}

char *ui_entry_text(const ui_entry *b)
{
	ui_entry_cr self = AX_R_INIT(ui_entry, b);
	if (b->mulitline)
		return uiEntryText(ENTRY(self));
	else
		return uiEntryText(ENTRY(self));
}

void ui_entry_set_text(ui_entry *b, const char *text)
{
	ui_entry_r self = AX_R_INIT(ui_entry, b);
	if (b->mulitline)
		uiMultilineEntrySetText(MENTRY(self), text);
	else
		uiEntrySetText(ENTRY(self), text);
}

void ui_entry_append(ui_entry *b, const char *text)
{
	ui_entry_r self = AX_R_INIT(ui_entry, b);
	if (b->mulitline)
		uiMultilineEntrySetText(MENTRY(self), text);
	else
		uiEntrySetText(ENTRY(self), text);
}

static void OnChanged(uiEntry *sender, void *arg)
{
	ui_entry_r self = { control_data(uiControl(sender)) };
	self.ui_entry->on_changed(self.ui_entry, arg);
}

static void OnMultilineChanged(uiMultilineEntry *sender, void *arg)
{
	ui_entry_r self = { control_data(uiControl(sender)) };
	self.ui_entry->on_changed(self.ui_entry, arg);
}

void ui_entry_on_changed(ui_entry *b, void (*f)(ui_entry *sender, void *arg), void *arg)
{
	ui_entry_r self = AX_R_INIT(ui_entry, b);
	if (b->mulitline)
		uiMultilineEntryOnChanged(MENTRY(self), f ? OnMultilineChanged : NULL, arg);
	else
		uiEntryOnChanged(ENTRY(self), f ? OnChanged : NULL, arg);
	self.ui_entry->on_changed = f;
}

bool ui_entry_readonly(const ui_entry *b)
{
	ui_entry_cr self = AX_R_INIT(ui_entry, b);
	if (b->mulitline)
		return uiMultilineEntryReadOnly(MENTRY(self));
	else
		return uiEntryReadOnly(ENTRY(self));
}

void ui_entry_set_readonly(ui_entry *b, bool readonly)
{
	ui_entry_r self = AX_R_INIT(ui_entry, b);
	if (b->mulitline)
		uiMultilineEntrySetReadOnly(MENTRY(self), readonly);
	else
		uiEntrySetReadOnly(ENTRY(self), readonly);
}

