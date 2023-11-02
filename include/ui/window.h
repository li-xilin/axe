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

#ifndef UI_WINDOW_H
#define UI_WINDOW_H
#include "widget.h"

#ifndef UI_WINDOW_DEFINED
#define UI_WINDOW_DEFINED
typedef struct ui_window_st ui_window;
#endif

#ifndef UI_SIZE_DEFINED
#define UI_SIZE_DEFINED
typedef struct ui_size_st ui_size;
#endif

#ifndef UI_POINT_DEFINED
#define UI_POINT_DEFINED
typedef struct ui_point_st ui_point;
#endif

#define ax_baseof_ui_window ui_widget
ax_concrete_declare(2, ui_window);
ax_concrete_creator(ui_window, const char *title, const ui_size *size, bool has_menu);

typedef void ui_window_event_f(ui_window *wnd, void *arg);

typedef bool ui_window_bool_event_f(ui_window *wnd, void *arg);

char *ui_window_title(const ui_window *wnd);
void ui_window_set_title(ui_window *wnd, const char *title);
void ui_window_pos(const ui_window *wnd, ui_point *pos);
void ui_window_set_pos(ui_window *wnd, const ui_point *pos);
void ui_window_clisize(const ui_window *wnd, ui_size *size);
void ui_window_set_clisize(ui_window *wnd, const ui_size *size);
bool ui_window_fullscreen(const ui_window *wnd);
void ui_window_set_fullscreen(ui_window *wnd, bool fullscreen);
bool ui_window_focused(const ui_window *wnd);
bool ui_window_borderless(const ui_window *wnd);
void ui_window_set_borderless(ui_window *wnd, bool borderless);
void ui_window_set_child(ui_window *wnd, ui_widget *child);
bool ui_window_margined(const ui_window *wnd);
void ui_window_set_margined(ui_window *wnd, bool margined);
bool ui_window_resizeable(const ui_window *wnd);
void ui_window_set_resizeable(ui_window *wnd, bool resizeable);
char *ui_show_openfile(ui_window *parent);
char *ui_show_openforder(ui_window *parent);
char *ui_show_savefile(ui_window *parent);
void ui_msgbox(ui_window *parent, const char *title, const char *description, bool error);
void ui_window_on_clisize_changed(ui_window *wnd, ui_window_event_f *handler, void *data);
void ui_window_on_moved(ui_window *wnd, ui_window_event_f *handler, void *data);
void ui_window_on_focus_changed(ui_window *wnd, ui_window_event_f *handler, void *data);
void ui_window_on_closing(ui_window *wnd, ui_window_bool_event_f *handler, void *data);
double ui_window_scale(ui_window *wnd);

#endif
