#include "ui/window.h"
#include "ui/ui.h"
#include "ui/types.h"
#include "ui/menu.h"
#include <stdio.h>

ui_window_r wnd;

bool on_closing(ui_window *sender, void *data)
{
	ui_quit();
	return true;
}

void file_on_clicked(ui_menu_item *mi, ui_window *w, void *arg)
{
	printf("file_open_clicked w = %s\n", ax_one_name(ax_r(ui_window, w).ax_one));
}

void word_wrap_on_clicked(ui_menu_item *mi, ui_window *w, void *arg)
{
	printf("word_wrap_on_clicked checked = %d\n", ui_menu_item_checked(mi));
}

void about_on_clicked(ui_menu_item *mi, ui_window *w, void *arg)
{
	ui_msgbox(w, "About", "Li Xilin <lixilin@gmx.com>", false);
}

bool on_should_quit(void *arg)
{
	ax_one_free(wnd.ax_one);
	return true;
}

int main()
{

	ui_init();
	ui_on_should_quit(on_should_quit, NULL);

	ui_menu_r m1 = ax_new(ui_menu, "File");
	ui_menu_r m2 = ax_new(ui_menu, "Edit");
	ui_menu_r m3 = ax_new(ui_menu, "Help");

	ui_menu_item *mi_open = ui_menu_append_item(m1.ui_menu, "Open");
	ui_menu_item_on_clicked(mi_open, file_on_clicked, NULL);

	ui_menu_append_quit(m1.ui_menu);

	ui_menu_append_preferences(m2.ui_menu);

	ui_menu_item *mi_word_wrap = ui_menu_append_checkitem(m2.ui_menu, "Word wrap");
	ui_menu_item_on_clicked(mi_word_wrap, word_wrap_on_clicked, NULL);

	ui_menu_item *mi_about = ui_menu_append_about(m3.ui_menu);
	ui_menu_item_on_clicked(mi_about, about_on_clicked, NULL);

	wnd = ax_new(ui_window, "Menu sample", ax_pstruct(ui_size, 400, 200), true);
	
	ui_window_on_closing(wnd.ui_window, on_closing, 0);
	ui_widget_show(wnd.ui_widget, true);

	ui_loop();

	ui_exit();

	return 0;
}

