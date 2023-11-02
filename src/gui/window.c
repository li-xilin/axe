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
#include "ui/ui.h"
#include "ui/window.h"
#include "ui/types.h"
#include "ax/detect.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

#define CONTROL(self) uiControl(ax_class_data(self.ui_widget).ctrl)
#define WINDOW(self) uiWindow(CONTROL(self))

#ifdef AX_OS_WIN
#include <windef.h>
#include <libloaderapi.h>
#include <winuser.h>

typedef WINAPI UINT (*GetDpiForWindowProc)(HWND);

static GetDpiForWindowProc _GetDpiForWindow = NULL;

static WINAPI UINT DefaultGetDpiForWindow(HWND hWnd)
{
        return 96;
}

#endif

ax_concrete_begin(ui_window)
	ui_window_event_f
		*on_focus_changed,
		*on_clisize_changed,
		*on_moved;
	ui_window_bool_event_f
		*on_closing;
ax_end;

typedef void window_event_f(uiWindow *sender, void *data);

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_window_r self = AX_R_INIT(ax_one, one);
	control_detach(CONTROL(self));
	uiControlDestroy(CONTROL(self));
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(2, ui_window);
}

const ui_widget_trait ui_window_tr =
{
	.ax_one = {
		.free = one_free,
		.name = one_name,
	},
};

ax_concrete_creator(ui_window, const char *title, const ui_size *size, bool has_menu)
{
        ui_widget *widget = NULL;
	uiWindow *wnd = NULL;

	wnd = uiNewWindow(title, ui_scale() * size->width, ui_scale() * size->height, has_menu);
	if (!wnd)
		goto fail;

        widget = malloc(sizeof(ui_window));
        if (!widget)
                goto fail;

	if (control_attach(uiControl(wnd), ax_r(ui_widget, widget).ax_one))
		goto fail;

#ifndef AX_OS_WIN
	if (!_GetDpiForWindow) {
		HMODULE hUser32 = GetModuleHandleA("user32.dll");

		if (hUser32)
			_GetDpiForWindow = (GetDpiForWindowProc)GetProcAddress(hUser32, "GetDpiForWindow");

		if (!_GetDpiForWindow)
			_GetDpiForWindow = &DefaultGetDpiForWindow;
	}
#endif

        ui_window window_init = {
		.ui_widget = {
			.tr = &ui_window_tr,
			.env.ctrl = wnd,
		},
        };

        memcpy(widget, &window_init, sizeof window_init);
        return widget;
fail:
        free(widget);
	uiFreeControl(uiControl(wnd));
        return NULL;
}

char *ui_window_title(const ui_window *wnd)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	return uiWindowTitle(WINDOW(self));
}

void ui_window_set_title(ui_window *wnd, const char *title)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetTitle(WINDOW(self), title);
}

void ui_window_pos(const ui_window *wnd, ui_point *pos)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	uiWindowPosition(WINDOW(self), &pos->x, &pos->y);
}

void ui_window_set_pos(ui_window *wnd, const ui_point *pos)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetPosition(WINDOW(self), pos->x, pos->y);
}

void ui_window_clisize(const ui_window *wnd, ui_size *size)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	uiWindowContentSize(WINDOW(self), &size->width, &size->height);
}

void ui_window_set_clisize(ui_window *wnd, const ui_size *size)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetContentSize(WINDOW(self), size->width, size->height);
}

bool ui_window_fullscreen(const ui_window *wnd)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	return uiWindowFullscreen(WINDOW(self));
}

void ui_window_set_fullscreen(ui_window *wnd, bool fullscreen)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetFullscreen(WINDOW(self), fullscreen);
}

bool ui_window_focused(const ui_window *wnd)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	return uiWindowFocused(WINDOW(self));
}

bool ui_window_borderless(const ui_window *wnd)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	return uiWindowBorderless(WINDOW(self));
}

void ui_window_set_borderless(ui_window *wnd, bool borderless)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetBorderless(WINDOW(self), borderless);
}

void ui_window_set_child(ui_window *wnd, ui_widget *child)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetChild(WINDOW(self), ax_class_data(child).ctrl);
}

bool ui_window_margined(const ui_window *wnd)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	return uiWindowMargined(WINDOW(self));
}

void ui_window_set_margined(ui_window *wnd, bool margined)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetMargined(WINDOW(self), margined);
}

bool ui_window_resizeable(const ui_window *wnd)
{
	ui_window_cr self = AX_R_INIT(ui_window, wnd);
	return uiWindowResizeable(WINDOW(self));
}

void ui_window_set_resizeable(ui_window *wnd, bool resizeable)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiWindowSetResizeable(WINDOW(self), resizeable);
}

char *ui_show_openfile(ui_window *parent)
{
	ui_window_r pwnd = AX_R_INIT(ui_window, parent);
	return uiOpenFile(WINDOW(pwnd));
}

char *ui_show_openforder(ui_window *parent)
{
	ui_window_r pwnd = AX_R_INIT(ui_window, parent);
	return uiOpenFolder(WINDOW(pwnd));
}

char *ui_show_savefile(ui_window *parent)
{
	ui_window_r pwnd = AX_R_INIT(ui_window, parent);
	return uiSaveFile(WINDOW(pwnd));
}

void ui_msgbox(ui_window *parent, const char *title, const char *description, bool error)
{
	ui_window_r pwnd = AX_R_INIT(ui_window, parent);
	if (error)
		uiMsgBox(WINDOW(pwnd), title, description);
	else
		uiMsgBoxError(WINDOW(pwnd), title, description);
}

static void OnWindowContentSizeChanged(uiWindow *sender, void *senderData)
{
	ui_window_r wnd = { control_data(uiControl(sender)) };
	wnd.ui_window->on_clisize_changed(wnd.ui_window, senderData);
}

static int OnWindowClosing(uiWindow *sender, void *senderData)
{
	ui_window_r wnd = { control_data(uiControl(sender)) };
	return wnd.ui_window->on_closing(wnd.ui_window, senderData);
}

static void OnWindowFocusChanged(uiWindow *sender, void *senderData)
{
	ui_window_r wnd = { control_data(uiControl(sender)) };
	wnd.ui_window->on_focus_changed(wnd.ui_window, senderData);
}

static void OnWindowPositionChanged(uiWindow *sender, void *senderData)
{
	ui_window_r wnd = { control_data(uiControl(sender)) };
	wnd.ui_window->on_moved(wnd.ui_window, senderData);
}

void ui_window_on_clisize_changed(ui_window *wnd, ui_window_event_f *handler, void *data)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	self.ui_window->on_clisize_changed = handler;
	uiWindowOnContentSizeChanged(WINDOW(self), handler ? OnWindowContentSizeChanged : NULL, data);
}

void ui_window_on_moved(ui_window *wnd, ui_window_event_f *handler, void *data)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	self.ui_window->on_moved = handler;
	uiWindowOnPositionChanged(WINDOW(self), handler ? OnWindowPositionChanged : NULL, data);
}

void ui_window_on_focus_changed(ui_window *wnd, ui_window_event_f *handler, void *data)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	self.ui_window->on_focus_changed = handler;
	uiWindowOnFocusChanged(WINDOW(self), handler ? OnWindowFocusChanged : NULL, data);
}

void ui_window_on_closing(ui_window *wnd, ui_window_bool_event_f *handler, void *data)
{
	ui_window_r self = AX_R_INIT(ui_window, wnd);
	self.ui_window->on_closing = handler;
	uiWindowOnClosing(WINDOW(self), handler ? OnWindowClosing : NULL, data);
}


double ui_window_scale(ui_window *wnd)
{
	double ratio = 1;
#ifdef AX_OS_WIN

	ui_window_r self = AX_R_INIT(ui_window, wnd);
	uiControl *ctl = ax_class_data(self.ui_widget).ctrl;
	HWND hWnd = (HWND)uiControlHandle(ctl);
	
	int zoom = _GetDpiForWindow(hWnd);
	switch (zoom) {
		case 96:
			ratio = 1;
			break;
		case 120:
			ratio = 1.25;
			break;
		case 144:
			ratio = 1.5;
			break;
		case 192:
			ratio = 2;
			break;
	}
#endif
	return ratio;
}

