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

#ifndef AX_MODEL_H
#define AX_MODEL_H

#include "ax/type/seq.h"

#ifndef UI_MODEL_DEFINED
#define UI_MODEL_DEFINED
typedef struct ui_model_st ui_model;
#endif

#ifndef UI_IMAGE_DEFINED
#define UI_IMAGE_DEFINED
typedef struct ui_image_st ui_image;
#endif

#ifndef UI_MODEL_RECORD_DEFINED
#define UI_MODEL_RECORD_DEFINED
typedef struct ui_model_record_st ui_model_record;
#endif

enum {
        UI_MODEL_TEXT,
        UI_MODEL_CHECKBOX,
        UI_MODEL_BUTTON,
        UI_MODEL_IMAGE,
        UI_MODEL_PROGRESS,
        UI_MODEL_CHECKBOX_TEXT,
        UI_MODEL_IMAGE_TEXT,
};

typedef struct ui_model_text_st 
{
	char *text;
	bool editable;
} ui_model_text;

typedef struct ui_model_checkbox_st 
{
	bool checked;
	bool checkable;
} ui_model_checkbox;

typedef struct ui_model_button_st 
{
	char *text;
	bool clickable;
} ui_model_button;

typedef struct ui_model_progress_st 
{
	char progress;
} ui_model_progress;

typedef struct ui_model_image_st 
{
	ui_image *image;
} ui_model_image;

typedef struct ui_model_checkbox_text_st 
{
	char *text;
	bool editable;
	bool checked;
	bool checkable;
} ui_model_checkbox_text;

typedef struct ui_model_image_text_st 
{
	char *text;
	ui_image *image;
	bool editable;
} ui_model_image_text;

#define ax_baseof_ui_model ax_seq
ax_concrete_declare(4, ui_model);

extern const ax_seq_trait ui_model_tr;

ax_concrete_creator(ui_model, int types[], size_t count);

ui_model_record *ui_model_record_alloc(ui_model *r);

void ui_model_record_free(ui_model_record *r, const ui_model *m);

void *ui_model_record_at(ui_model_record *r, const ui_model *m, int column);

size_t ui_model_column_count(const ui_model *m);

void *ui_model_raw_model(ui_model *m);

void ui_model_on_clicked(ui_model *m, void (*f)(ui_model *sender, int column, int row, void *arg), void *arg);

void ui_model_on_checked(ui_model *m, bool (*f)(ui_model *sender, int column, int row, bool new_checked, void *arg), void *arg);

void ui_model_on_changed(ui_model *m, bool (*f)(ui_model *sender, int column, int row, const char *new_string, void *arg), void *arg);

void ui_model_update(ui_model *m, int index);
#endif
