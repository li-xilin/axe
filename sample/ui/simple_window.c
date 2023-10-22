#include "ui/window.h"
#include "ui/ui.h"
#include "ui/types.h"
#include <stdio.h>

bool on_closing(ui_window *sender, void *data)
{
	printf("closing\n");
	ui_quit();
	return true;
}

void on_move(ui_window *sender, void *data)
{
	ui_point pos;
	ui_window_pos(sender, &pos);
	printf("new pos %d, %d\n", pos.x, pos.y);
}

void on_size_changed(ui_window *sender, void *data)
{
	ui_size size;
	ui_window_clisize(sender, &size);
	printf("new size %d, %d\n", size.width, size.height);
}

int main()
{
	ui_init();

	ui_size size = { 500, 500 };
	ui_window_r wnd = ax_new(ui_window, "hello", &size, 0);

	ui_widget_show(wnd.ui_widget, true);
	ui_window_on_closing(wnd.ui_window, on_closing, 0);
	ui_window_on_moved(wnd.ui_window, on_move, 0);
	ui_window_on_clisize_changed(wnd.ui_window, on_size_changed, 0);

	ui_loop();

	return 0;
}

