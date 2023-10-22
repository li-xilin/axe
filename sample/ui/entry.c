#include "ui/window.h"
#include "ui/form.h"
#include "ui/ui.h"
#include "ui/entry.h"
#include "ui/types.h"
#include <stdio.h>

bool on_closing(ui_window *sender, void *data)
{
        ui_quit();
        return true;
}

int main()
{
	ui_init();

	ui_size size = { 500, 500 };
	ui_window_r wnd = ax_new(ui_window, "Entries", &size, 0);
	ui_window_on_closing(wnd.ui_window, on_closing, NULL);
	ui_window_set_margined(wnd.ui_window, true);

	ui_form_r form = ax_new0(ui_form);
	ui_form_set_padded(form.ui_form, true);

	ui_entry_r entry1 = ax_new(ui_entry, UI_ENTRY_NORMAL, "Entry1");
	ui_entry_r entry2 = ax_new(ui_entry, UI_ENTRY_SEARCH, "Entry2");
	ui_entry_r entry3 = ax_new(ui_entry, UI_ENTRY_PASSWORD, "Entry3");
	ui_entry_r entry4 = ax_new(ui_entry, UI_ENTRY_SCROLLABLE, "Entry3");
	ui_entry_r entry5 = ax_new(ui_entry, UI_ENTRY_VSCROLLABLE, "Entry3");

	ui_form_append(form.ui_form, "Entry1", entry1.ui_widget, false);
	ui_form_append(form.ui_form, "Entry2", entry2.ui_widget, false);
	ui_form_append(form.ui_form, "Entry3", entry3.ui_widget, false);
	ui_form_append(form.ui_form, "Entry4", entry4.ui_widget, true);
	ui_form_append(form.ui_form, "Entry5", entry5.ui_widget, true);

	ui_window_set_child(wnd.ui_window, form.ui_widget);

	ui_widget_show(wnd.ui_widget, true);
	ui_loop();

	return 0;
}
