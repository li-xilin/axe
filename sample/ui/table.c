#include "ax/log.h"
#include "ax/dump.h"
#include "ui/table.h"
#include "ui/model.h"
#include "ui/button.h"
#include "ui/window.h"
#include "ui/box.h"
#include "ui/ui.h"
#include "ui/label.h"
#include "ui/image.h"
#include "ui/types.h"
#include <stdio.h>
#include <stdlib.h>

bool on_closing(ui_window *sender, void *data)
{
        printf("closing ...\n");
        ui_quit();
        return true;
}

static ui_image_r create_image()
{
	int data[10][10] = {
		{ 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, },
		{ 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, },
		{ 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, },
		{ 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, },
		{ 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, },
		{ 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, },
		{ 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, },
		{ 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, },
		{ 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, },
		{ 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0xFF0000FF, 0xFF0000FF, 0x00000000, 0x00000000, 0x00000000, 0x00000000, },
	};
	ui_image_r img = ax_new(ui_image, 10, 10);
	ui_image_load(img.ui_image, data, 10, 10, 40);
	return img;
}

static ui_model_record *create_record(ui_model_r model, const char *name)
{
	ui_model_record *r = ui_model_record_alloc(model.ui_model);

	ui_model_text *text = ui_model_record_at(r, model.ui_model, 0);
	text->editable = true;
	text->text = ax_strdup(name);

	ui_model_button *button = ui_model_record_at(r, model.ui_model, 1);
	button->text = ax_strdup(name);
	button->clickable = true;

	ui_model_checkbox *checkbox = ui_model_record_at(r, model.ui_model, 2);
	checkbox->checked = true;
	checkbox->checkable = true;

	ui_model_progress *progress = ui_model_record_at(r, model.ui_model, 3);
	progress->progress = 50;

	ui_model_image *image = ui_model_record_at(r, model.ui_model, 4);
	image->image = create_image().ui_image;

	ui_model_checkbox_text *checkbox_text = ui_model_record_at(r, model.ui_model, 5);
	checkbox_text->checked = true;
	checkbox_text->checkable = true;
	checkbox_text->text = ax_strdup(name);
	checkbox_text->editable = true;

	ui_model_image_text *image_text = ui_model_record_at(r, model.ui_model, 6);
	image_text->text = ax_strdup(name);
	image_text->editable = true;
	image_text->image = create_image().ui_image;

	return r;
}

static void table_remove(ui_model_r model, int index)
{
	ax_iter it = ax_seq_at(model.ax_seq, index);
	ax_iter end = ax_box_end(model.ax_box);
	if (ax_iter_equal(&it, &end)) {
		ax_perror("Invalid index");
		return;
	}
	ax_iter_erase(&it);
}

static void button1_on_clicked(ui_button *sender, void *arg)
{
	ui_table *t = arg;
	ui_model_r m = ui_table_get_model(t);
	size_t count;
	int *rows = ui_table_selected(t, &count);

	char buf[100];

	if (count == 0) {
		int index = ax_box_size(m.ax_box);
		sprintf(buf, "Text %d", index);
		struct ui_model_record_st *r = create_record(m, buf);
		ax_seq_push(m.ax_seq, r);
	}
	else {
		sprintf(buf, "%d", rows[0]);
		struct ui_model_record_st *r = create_record(m, buf);
		ax_iter it = ax_seq_at(m.ax_seq, rows[0]);
		ax_seq_insert(m.ax_seq, &it, r);
	}
	free(rows);
}

static void button2_on_clicked(ui_button *sender, void *arg)
{
	ui_table *t = arg;
	size_t count;
	int *rows = ui_table_selected(t, &count);
	if (!count)
		goto out;

	table_remove(ui_table_get_model(t), rows[0]);
out:
	free(rows);
}

static void button3_on_clicked(ui_button *sender, void *arg)
{
	ui_table *t = arg;
	ui_model_r m = ui_table_get_model(t);

	ax_box_foreach(m.ax_box, ui_model_record *, row) {
		ui_model_text *t = ui_model_record_at(row, m.ui_model, 0);
		ui_model_button *b = ui_model_record_at(row, m.ui_model, 1);
		ui_model_checkbox *c = ui_model_record_at(row, m.ui_model, 2);
		ui_model_progress *p = ui_model_record_at(row, m.ui_model, 3);
		ui_model_image *i = ui_model_record_at(row, m.ui_model, 4);
		ui_model_checkbox_text *ct = ui_model_record_at(row, m.ui_model, 5);
		ui_model_image_text *it = ui_model_record_at(row, m.ui_model, 6);
		printf("| '%s' | '%s' | %d | %d% | %p | %d '%s' | %p '%s' |\n", t->text, b->text, c->checked, p->progress, \
				(void *)i->image, ct->checked, ct->text, (void *)it->image, it->text);
	}

	putchar('\n');
}

static void button4_on_clicked(ui_button *sender, void *arg)
{
	ui_table *t = arg;
	ui_model_r m = ui_table_get_model(t);

	ax_dump_fput(ax_any_dump(m.ax_any), ax_dump_pretty_format(), stderr);
	// ax_dump_out(m.ax_any);
}

static void table_on_clicked(ui_table *sender, int row, void *arg)
{
	printf("on_clicked row = %d\n", row);
}

static void table_on_double_clicked(ui_table *sender, int row, void *arg)
{
	printf("on_double_clicked row = %d\n", row);
}

static void table_on_selected(ui_table *sender, void *arg)
{
	printf("on_selected\n");
}

static void model_on_clicked(ui_model *sender, int column, int row, void *arg)
{
	printf("model_on_clicked column = %d, row = %d\n", column, row);
}

static bool model_on_changed(ui_model *sender, int column, int row, const char *new_str, void *arg)
{
	printf("model_on_changed column = %d, row = %d, new_str = %s\n", column, row, new_str);
	return true;
}

static bool model_on_checked(ui_model *sender, int column, int row, bool checked, void *arg)
{
	printf("model_on_checked column = %d, row = %d, checked = %d\n", column, row, checked);
	return true;
}


int main()
{
        ui_init();

        ui_size size = { 800, 400 };
        ui_window_r wnd = ax_new(ui_window, "Table sample", &size, 0);
	ui_window_set_margined(wnd.ui_window, true);

        ui_window_on_closing(wnd.ui_window, on_closing, 0);

        ui_box_r vbox = ax_new(ui_box, UI_BOX_VERTICAL);
        ui_box_r hbox = ax_new(ui_box, UI_BOX_HORIZONTAL);
	ui_box_set_padded(vbox.ui_box, true);
	ui_box_set_padded(hbox.ui_box, true);

	ui_table_header headers[] = {
		{ "A", UI_MODEL_TEXT },
		{ "B", UI_MODEL_BUTTON },
		{ "C", UI_MODEL_CHECKBOX },
		{ "D", UI_MODEL_PROGRESS },
		{ "E", UI_MODEL_IMAGE },
		{ "F", UI_MODEL_CHECKBOX_TEXT },
		{ "G", UI_MODEL_IMAGE_TEXT },
		{ NULL, 0 }
	};
	ui_table_r t = ax_new(ui_table, headers);
	ui_table_set_column_width(t.ui_table, 0, 80);
	ui_table_set_column_width(t.ui_table, 1, 80);
	ui_table_set_column_width(t.ui_table, 2, 80);
	ui_table_set_column_width(t.ui_table, 3, 80);
	ui_table_set_column_width(t.ui_table, 4, 80);
	ui_table_set_column_width(t.ui_table, 5, 80);
	ui_table_set_column_width(t.ui_table, 6, 80);

	ui_table_on_clicked(t.ui_table, table_on_clicked, NULL);
	ui_table_on_double_clicked(t.ui_table, table_on_double_clicked, NULL);
	ui_table_on_selected(t.ui_table, table_on_selected, NULL);

	ui_model_r m = ui_table_get_model(t.ui_table);
	ui_model_on_changed(m.ui_model, model_on_changed, NULL);
	ui_model_on_checked(m.ui_model, model_on_checked, NULL);
	ui_model_on_clicked(m.ui_model, model_on_clicked, NULL);
	ax_one_free(m.ax_one);

        ui_box_append(hbox.ui_box, t.ui_widget, true);
        ui_box_append(hbox.ui_box, vbox.ui_widget, false);

        ui_button_r button1 = ax_new(ui_button, "Insert or append item");
	ui_button_on_clicked(button1.ui_button, button1_on_clicked, t.ax_one);
        ui_button_r button2 = ax_new(ui_button, "Remove selected item");
	ui_button_on_clicked(button2.ui_button, button2_on_clicked, t.ax_one);
        ui_button_r button3 = ax_new(ui_button, "Print model data");
	ui_button_on_clicked(button3.ui_button, button3_on_clicked, t.ax_one);
        ui_button_r button4 = ax_new(ui_button, "Dump model data");
	ui_button_on_clicked(button4.ui_button, button4_on_clicked, t.ax_one);

        ui_box_append(vbox.ui_box, button1.ui_widget, false);
        ui_box_append(vbox.ui_box, button2.ui_widget, false);
        ui_box_append(vbox.ui_box, button3.ui_widget, false);
        ui_box_append(vbox.ui_box, button4.ui_widget, false);

        ui_window_set_child(wnd.ui_window, hbox.ui_widget);

        ui_widget_show(wnd.ui_widget, true);
        ui_loop();

        return 0;
}
