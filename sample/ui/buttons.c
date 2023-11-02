#include "ui/window.h"
#include "ui/form.h"
#include "ui/ui.h"
#include "ui/button.h"
#include "ui/colorpicker.h"
#include "ui/timepicker.h"
#include "ui/types.h"
#include <stdio.h>

static bool wnd_on_closing(ui_window *sender, void *data)
{
        ui_quit();
        return true;
}

static void button1_on_click(ui_button *sender, void *arg)
{
	printf("button1 clicked\n");
}

static void button2_3_4_on_changed(ui_timepicker *sender, void *arg)
{
	struct tm t;
	ui_timepicker_value(sender, &t);
	printf("button%d %s\n", *(int *)arg, asctime(&t));
}

static void button5_on_changed(ui_colorpicker *sender, void *arg)
{
	ui_rgba c;
	ui_colorpicker_value(sender, &c);
	printf("button5 changed, color = %hhu, %hhu, %hhu, %hhu\n", c.r, c.g, c.b, c.a);
}

int main()
{
	ui_init();

	ui_size size = { 400, 600 };
	ui_window_r wnd = ax_new(ui_window, "Buttons", &size, 0);
	ui_window_on_closing(wnd.ui_window, wnd_on_closing, NULL);
	ui_window_set_margined(wnd.ui_window, true);

	ui_form_r form = ax_new0(ui_form);
	ui_form_set_padded(form.ui_form, true);

	ui_button_r button1 = ax_new(ui_button, "button1");
	ui_timepicker_r button2 = ax_new(ui_timepicker, UI_TIMEPICKER_DATE);
	ui_timepicker_r button3 = ax_new(ui_timepicker, UI_TIMEPICKER_TIME);
	ui_timepicker_r button4 = ax_new(ui_timepicker, UI_TIMEPICKER_DATETIME);
	ui_colorpicker_r button5 = ax_new(ui_colorpicker, ax_pstruct(ui_rgba, 0x00, 0x00, 0xFF, 0xFF));

	ui_button_on_clicked(button1.ui_button, button1_on_click, NULL);
	ui_timepicker_on_changed(button2.ui_timepicker, button2_3_4_on_changed, ax_p(int, 2));
	ui_timepicker_on_changed(button3.ui_timepicker, button2_3_4_on_changed, ax_p(int, 3));
	ui_timepicker_on_changed(button4.ui_timepicker, button2_3_4_on_changed, ax_p(int, 4));
	ui_colorpicker_on_changed(button5.ui_colorpicker, button5_on_changed, NULL);

	ui_form_append(form.ui_form, "button1", button1.ui_widget, false);
	ui_form_append(form.ui_form, "button2", button2.ui_widget, false);
	ui_form_append(form.ui_form, "button3", button3.ui_widget, false);
	ui_form_append(form.ui_form, "button4", button4.ui_widget, false);
	ui_form_append(form.ui_form, "button5", button5.ui_widget, false);

	ui_window_set_child(wnd.ui_window, form.ui_widget);

	ui_widget_show(wnd.ui_widget, true);
	ui_loop();

	return 0;
}
