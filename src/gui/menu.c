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

#include "ui/menu.h"
#include "control.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

ax_concrete_begin(ui_menu)
	uiMenu *menu;
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	// ui_menu_r self = AX_R_INIT(ax_one, one);
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(1, ui_menu);
}

const ax_one_trait ui_menu_tr =
{
	.free = one_free,
	.name = one_name,
};

ax_concrete_creator(ui_menu, const char *name)
{
        ax_one *one = NULL;
	uiMenu *menu = NULL;

	menu = uiNewMenu(name);
	if (!menu)
		goto fail;

        one = malloc(sizeof(ui_menu));
        if (!one)
                goto fail;

        ui_menu menu_init = {
		.ax_one = {
			.tr = &ui_menu_tr,
		},
		.menu = menu,
        };

        memcpy(one, &menu_init, sizeof menu_init);
        return one;
fail:
        free(one);
        return NULL;
}

ui_menu_item *ui_menu_append_item(ui_menu *m, const char *text)
{
	return (ui_menu_item *)uiMenuAppendItem(m->menu, text);
}

ui_menu_item *ui_menu_append_checkitem(ui_menu *m, const char *text)
{
	return (ui_menu_item *)uiMenuAppendCheckItem(m->menu, text);
}

ui_menu_item *ui_menu_append_preferences(ui_menu *m)
{
	return (ui_menu_item *)uiMenuAppendPreferencesItem(m->menu);
}

ui_menu_item *ui_menu_append_about(ui_menu *m)
{
	return (ui_menu_item *)uiMenuAppendAboutItem(m->menu);
}

void ui_menu_append_quit(ui_menu *m)
{
	uiMenuAppendQuitItem(m->menu);
}

void ui_menu_append_separator(ui_menu *m)
{
	uiMenuAppendSeparator(m->menu);
}

struct data_on_clicked_st
{
	void *arg;
	void (*on_clicked)(ui_menu_item *mi, ui_window *w, void *arg);
};

static void OnClick(uiMenuItem *mi, uiWindow *w, void *arg)
{
	struct data_on_clicked_st *ctx = arg;
	ui_window *wnd = (ui_window *)control_data(uiControl(w));
	ctx->on_clicked((ui_menu_item *)mi, wnd, ctx->arg);
}

int ui_menu_item_on_clicked(ui_menu_item *mi, void (*f)(ui_menu_item *mi, ui_window *w, void *arg), void *arg)
{
	struct data_on_clicked_st *ctx = malloc(sizeof *ctx);
	if (!ctx)
		return -1;
	ctx->arg = arg;
	ctx->on_clicked = f;
	uiMenuItemOnClicked((uiMenuItem *)mi, OnClick, ctx);
	return 0;
}

void ui_menu_item_set_enable(ui_menu_item *mi, bool enable)
{
	if (enable)
		uiMenuItemEnable((uiMenuItem *)mi);
	else
		uiMenuItemDisable((uiMenuItem *)mi);
}

void ui_menu_item_set_checked(ui_menu_item *mi, bool checked)
{
	uiMenuItemSetChecked((uiMenuItem *)mi, checked);
}

bool ui_menu_item_checked(const ui_menu_item *mi)
{
	return uiMenuItemChecked((uiMenuItem *)mi);
}

