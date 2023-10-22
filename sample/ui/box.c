#include "ui/window.h"
#include "ui/box.h"
#include "ui/ui.h"
#include "ui/label.h"
#include "ui/types.h"
#include <stdio.h>

bool on_closing(ui_window *sender, void *data)
{
	printf("closing ...\n");
	ui_quit();
	return true;
}

int main()
{
	ui_init();

	ui_size size = { 200, 200 };
	ui_window_r wnd = ax_new(ui_window, "hello", &size, 0);

	ui_window_on_closing(wnd.ui_window, on_closing, 0);

	ui_box_r vbox = ax_new(ui_box, UI_BOX_VERTICAL);
	ui_box_r hbox = ax_new(ui_box, UI_BOX_HORIZONTAL);


	ui_label_r label1 = ax_new(ui_label, "Label1");
	ui_label_r label2 = ax_new(ui_label, "Label2");
	ui_label_r label3 = ax_new(ui_label, "Label3");

	ui_box_append(hbox.ui_box, label1.ui_widget, true);
	ui_box_append(hbox.ui_box, label2.ui_widget, true);

	ui_box_append(vbox.ui_box, hbox.ui_widget, true);
	ui_box_append(vbox.ui_box, label3.ui_widget, true);

	ui_window_set_child(wnd.ui_window, vbox.ui_widget);

	ui_widget_show(wnd.ui_widget, true);
	ui_loop();

	return 0;
}
