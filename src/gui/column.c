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

#include "ui/model.h"
#include "ui/image.h"
#include "ax/log.h"
#include <ui.h>

#include <stdlib.h>

uiTableValue *column_text_get(ui_model_text *column, int who)
{
	switch (who) {
		case 0: return uiNewTableValueString(column->text);
		case 1: return uiNewTableValueInt(column->editable);
		default: return NULL;
	}
}

void column_text_set(ui_model_text *column, int who, const uiTableValue *val)
{
	switch (who) {
		case 0:
			free(column->text);
			column->text = ax_strdup(uiTableValueString(val));
			break;
		case 1:
			column->editable = uiTableValueInt(val);
			break;
	}
}


uiTableValue *column_checkbox_get(ui_model_checkbox *column, int who)
{
	switch (who) {
		case 0: return uiNewTableValueInt(column->checked);
		case 1: return uiNewTableValueInt(column->checkable);
		default: return NULL;
	}
}

void column_checkbox_set(ui_model_checkbox *column, int who, const uiTableValue *val)
{
	switch (who) {
		case 0:
			column->checked = uiTableValueInt(val);
			break;
		case 1:
			column->checkable = uiTableValueInt(val);
			break;
	}
}

uiTableValue *column_button_get(ui_model_button *column, int who)
{
	switch (who) {
		case 0: return uiNewTableValueString(column->text);
		case 1: return uiNewTableValueInt(column->clickable);
		default: return NULL;
	}
}

void column_button_set(ui_model_button *column, int who, const uiTableValue *val)
{
	switch (who) {
		case 0:
			if (val) {
				free(column->text);
				column->text = ax_strdup(uiTableValueString(val));
			}
			break;
		case 1: 
			column->clickable = uiTableValueInt(val);
			break;
	}
}

uiTableValue *column_progress_get(ui_model_progress *column, int who)
{
	switch (who) {
		case 0: return uiNewTableValueInt(column->progress);
		default: return NULL;
	}
}

void column_progress_set(ui_model_progress *column, int who, const uiTableValue *val)
{
	switch (who) {
		case 0:
			column->progress = uiTableValueInt(val);
			break;
	}
}

uiTableValue *column_image_get(ui_model_image *column, int who)
{
	switch (who) {
		case 0: return uiNewTableValueImage(ui_image_raw_image(column->image));
		default: return NULL;
	}
}

void column_image_set(ui_model_image *column, int who, const uiTableValue *val)
{
	
}

uiTableValue *column_checkbox_text_get(ui_model_checkbox_text *column, int who)
{
	switch (who) {
		case 0: return uiNewTableValueInt(column->checked);
		case 1: return uiNewTableValueInt(column->checkable);
		case 2: return uiNewTableValueString(column->text);
		case 3: return uiNewTableValueInt(column->editable);
		default: return NULL;
	}
}

void column_checkbox_text_set(ui_model_checkbox_text *column, int who, const uiTableValue *val)
{
	switch (who) {
		case 0:
			column->checked = uiTableValueInt(val);
			break;
		case 1:
			column->checkable = uiTableValueInt(val);
			break;

		case 2:
			free(column->text);
			column->text = ax_strdup(uiTableValueString(val));
			break;
		case 3:
			column->editable = uiTableValueInt(val);
			break;
	}
}


uiTableValue *column_image_text_get(ui_model_image_text *column, int who)
{
	switch (who) {
		case 0: return uiNewTableValueImage(ui_image_raw_image(column->image));
		case 1: return uiNewTableValueString(column->text);
		case 2: return uiNewTableValueInt(column->editable);
		default: return NULL;
	}
}

void column_image_text_set(ui_model_image_text *column, int who, const uiTableValue *val)
{
	switch (who) {
		case 0:
			break;
		case 1:
			free(column->text);
			column->text = ax_strdup(uiTableValueString(val));
			break;
		case 2:
			column->editable = uiTableValueInt(val);
			break;
	}
}

size_t column_size(int type)
{
	switch (type) {
		case UI_MODEL_TEXT:
			return sizeof(char *) + sizeof(bool);
		case UI_MODEL_CHECKBOX:
			return sizeof(bool) + sizeof(bool);
		case UI_MODEL_BUTTON:
			return sizeof(char *) + sizeof(bool);
		case UI_MODEL_PROGRESS:
			return sizeof(char);
		case UI_MODEL_IMAGE:
			return sizeof(void *);
		case UI_MODEL_CHECKBOX_TEXT:
			return sizeof(char *) + sizeof(bool) * 3;
		case UI_MODEL_IMAGE_TEXT:
			return sizeof(char *) * 2 + sizeof(bool);
		default:
			ax_perror("bad column type %d", type);
	};
	return 0;
}

uiTableValueType column_type(int type, int who)
{
	switch (type) {
		case UI_MODEL_TEXT:
			switch (who) {
				case 0: return uiTableValueTypeString;
				case 1: return uiTableValueTypeInt;
			}
		case UI_MODEL_CHECKBOX:
			return uiTableValueTypeInt;
		case UI_MODEL_BUTTON:
			switch (who) {
				case 0: return uiTableValueTypeString;
				case 1: return uiTableValueTypeInt;
			}
		case UI_MODEL_PROGRESS:
			return uiTableValueTypeInt;
		case UI_MODEL_IMAGE:
			return uiTableValueTypeImage;
		case UI_MODEL_CHECKBOX_TEXT:
			switch (who) {
				case 0: return uiTableValueTypeInt;
				case 1: return uiTableValueTypeInt;
				case 2: return uiTableValueTypeString;
				case 3: return uiTableValueTypeInt;
			}
		case UI_MODEL_IMAGE_TEXT:
			switch (who) {
				case 0: return uiTableValueTypeImage;
				case 1: return uiTableValueTypeString;
				case 2: return uiTableValueTypeInt;
			}
		default:
			ax_perror("bad column type %d", type);
	};
	return 0;
}

