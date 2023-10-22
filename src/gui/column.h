#ifndef COLUMN_H
#define COLUMN_H

#include <ui.h>
#include "ui/model.h"

uiTableValue *column_text_get(ui_model_text *column, int who);

void column_text_set(ui_model_text *column, int who, const uiTableValue *val);

uiTableValue *column_checkbox_get(ui_model_checkbox *column, int who);

void column_checkbox_set(ui_model_checkbox *column, int who, const uiTableValue *val);

uiTableValue *column_button_get(ui_model_button *column, int who);

void column_button_set(ui_model_button *column, int who, const uiTableValue *val);

uiTableValue *column_progress_get(ui_model_progress *column, int who);

void column_progress_set(ui_model_progress *column, int who, const uiTableValue *val);

uiTableValue *column_image_get(ui_model_image *column, int who);

void column_image_set(ui_model_image *column, int who, const uiTableValue *val);

uiTableValue *column_checkbox_text_get(ui_model_checkbox_text *column, int who);

void column_checkbox_text_set(ui_model_checkbox_text *column, int who, const uiTableValue *val);

uiTableValue *column_image_text_get(ui_model_image_text *column, int who);

void column_image_text_set(ui_model_image_text *column, int who, const uiTableValue *val);

size_t column_size(int type);

uiTableValueType column_type(int type, int who);

#endif
