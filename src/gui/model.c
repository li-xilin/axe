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

#include "column.h"
#include "ui/model.h"
#include "ui/types.h"
#include "ax/vector.h"
#include "ax/def.h"
#include "ax/iter.h"
#include "ax/trait.h"
#include "ax/mem.h"
#include "ax/dump.h"
#include "ax/log.h"
#include "check.h"

#include <ui.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#define MIN_SIZE
#define ELEM_SIZE(b) ax_trait_size(ax_class_data(b.ax_box).elem_tr)
#undef free

#define HANDLER_MODEL(h) (*(ui_model **)((uint8_t *)h + sizeof(uiTableModelHandler)))

typedef union {
	ui_model_text *text;
	ui_model_image *image;
	ui_model_button *button;
	ui_model_checkbox *checkbox;
	ui_model_progress *progress;
	ui_model_image_text *image_text;
	ui_model_checkbox_text *checkbox_text;
} field_union;

ax_concrete_begin(ui_model)
	ax_vector_r rc_tab;
	uiTableModel *uimodel;
	uiTableModelHandler *handler;
	void (*on_clicked)(ui_model *sender, int column, int row, void *arg);
	void *data_on_clicked;
	bool (*on_checked)(ui_model *sender, int column, int row, bool new_checked, void *arg);
	void *data_on_checked;
	bool (*on_changed)(ui_model *sender, int column, int row, const char *new_value, void *arg);
	void *data_on_changed;
	size_t column_count;
	char *type_tab;
	size_t *column_off_tab;
ax_end;

struct ui_model_record_st
{
	ui_model *m;
	ui_rgba bg_color;
	char data[];
};

static void free_field(void *field_ptr, int type);

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap);
static ax_fail seq_pop(ax_seq *seq);
static void    seq_invert(ax_seq *seq);
static ax_fail seq_trunc(ax_seq *seq, size_t size);
static ax_iter seq_at(const ax_seq *seq, size_t index);
static void   *seq_last(const ax_seq *seq);
static void   *seq_first(const ax_seq *seq);

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap);

static size_t  box_size(const ax_box *box);
static size_t  box_maxsize(const ax_box *box);
static ax_iter box_begin(ax_box *box);
static ax_iter box_end(ax_box *box);
static ax_iter box_rbegin(ax_box *box);
static ax_iter box_rend(ax_box *box);
static void    box_clear(ax_box *box);

static ax_any *any_copy(const ax_any *any);

static void    one_free(ax_one *one);
static const char *one_name(const ax_one *one);

static void    citer_move(ax_citer *it, long i);
static void    citer_prev(ax_citer *it);
static void    citer_next(ax_citer *it);
static bool    citer_less(const ax_citer *it1, const ax_citer *it2);
static long    citer_dist(const ax_citer *it1, const ax_citer *it2);
static ax_box *citer_box(const ax_citer *it);

static void    rciter_move(ax_citer *it, long i);
static void    rciter_prev(ax_citer *it);
static void    rciter_next(ax_citer *it);
static long    rciter_dist(const ax_citer *it1, const ax_citer *it2);

static void   *citer_get(const ax_citer *it);
static void    iter_erase(ax_iter *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);

static void citer_move(ax_citer *it, long i)
{
	ui_model_cr self = AX_R_INIT(ax_one, it->owner);
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->iter;
	it->tr->move(it, i);
	it->owner = self.ax_seq;
	it->tr = &ui_model_tr.ax_box.iter;
}

static void citer_prev(ax_citer *it)
{
	ui_model_cr self = AX_R_INIT(ax_one, it->owner);
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->iter;
	it->tr->prev(it);
	it->owner = self.ax_seq;
	it->tr = &ui_model_tr.ax_box.iter;
}

static void citer_next(ax_citer *it)
{
	ui_model_cr self = AX_R_INIT(ax_one, it->owner);
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->iter;
	it->tr->next(it);
	it->owner = self.ax_seq;
	it->tr = &ui_model_tr.ax_box.iter;
}

static bool citer_less(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	ax_citer it1_tmp = *it1, it2_tmp = *it2;
	ui_model_cr self = AX_R_INIT(ax_one, it1->owner);
	it1_tmp.owner = it2_tmp.owner = self.ui_model->rc_tab.ax_vector;
	it1_tmp.tr = it2_tmp.tr = &ui_model_tr.ax_box.iter;
	return it1->tr->less(&it1_tmp, &it2_tmp);
}

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	ax_citer it1_tmp = *it1, it2_tmp = *it2;
	ui_model_cr self = AX_R_INIT(ax_one, it1->owner);
	it1_tmp.owner = it2_tmp.owner = self.ui_model->rc_tab.ax_vector;
	it1_tmp.tr = it2_tmp.tr = &ui_model_tr.ax_box.iter;
	long dist = it1_tmp.tr->dist(&it1_tmp, &it2_tmp);
	return it1_tmp.tr->norm ? dist : - dist;
}

static ax_box *citer_box(const ax_citer *it)
{
	ax_vector_cr self = { it->owner };
	return (ax_box *)self.ax_box;
}

static void rciter_move(ax_citer *it, long i)
{
	ui_model_cr self = AX_R_INIT(ax_one, it->owner);
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->riter;
	it->tr->move(it, i);
	it->owner = self.ui_model;
	it->tr = &ui_model_tr.ax_box.riter;
}

static void rciter_prev(ax_citer *it)
{
	ui_model_cr self = AX_R_INIT(ax_one, it->owner);
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->riter;
	it->tr->prev(it);
	it->owner = self.ui_model;
	it->tr = &ui_model_tr.ax_box.riter;
}

static void rciter_next(ax_citer *it)
{
	ui_model_cr self = AX_R_INIT(ax_one, it->owner);
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->riter;
	it->tr->next(it);
	it->owner = self.ui_model;
	it->tr = &ui_model_tr.ax_box.riter;
}

static void *citer_get(const ax_citer *it)
{
	ax_citer it_tmp = *it;
	ui_model_cr self = { it->owner };
	it_tmp.owner = self.ui_model->rc_tab.ax_vector;
	it_tmp.tr = &self.ui_model->rc_tab.ax_box->tr->iter;
	return it_tmp.tr->get(&it_tmp);
}

static long rciter_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_ITER_COMPARABLE(it1, it2);
	ax_citer it1_tmp = *it1, it2_tmp = *it2;
	ui_model_cr self = AX_R_INIT(ax_one, it1->owner);
	it1_tmp.owner = it2_tmp.owner = self.ui_model->rc_tab.ax_vector;
	it1_tmp.tr = it2_tmp.tr = &self.ui_model->rc_tab.ax_box->tr->riter;
	return it1_tmp.tr->dist(&it1_tmp, &it2_tmp);
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	ui_model_r self = { it->owner };
	ax_iter it_tmp = *it;
	it_tmp.owner = self.ui_model->rc_tab.ax_vector;
	it_tmp.tr = &self.ui_model->rc_tab.ax_box->tr->iter;

	ax_fail fail = it->tr->set(&it_tmp, val, ap);
	if (fail)
		return fail;

	ax_iter begin = it->tr->norm ? ax_box_begin(self.ax_box) : ax_box_rbegin(self.ax_box);
	begin.point = self.ui_model->rc_tab.ax_vector;
	long dist = it->tr->dist(ax_iter_cc(&begin), ax_iter_cc(it));

	uiTableModelRowChanged(self.ui_model->uimodel, it->tr->norm ? dist : (box_size(self.ax_box) - dist - 1));

	return fail;
}

static void iter_erase(ax_iter *it)
{
	ui_model_r self = AX_R_INIT(ax_one, it->owner);
	size_t offset = ((uint8_t *)it->point - (uint8_t *)ax_vector_buffer(self.ui_model->rc_tab.ax_vector)) / self.ax_box->env.elem_tr->t_size;
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->iter;
	it->tr->erase(it);
	it->owner = self.ui_model;
	it->tr = &ui_model_tr.ax_box.iter;
	uiTableModelRowDeleted(self.ui_model->uimodel, offset);
}

static void riter_erase(ax_iter *it)
{
	ui_model_r self = AX_R_INIT(ax_one, it->owner);
	it->owner = self.ui_model->rc_tab.ax_vector;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->riter;
	it->tr->erase(it);
	it->owner = self.ui_model;
	it->tr = &ui_model_tr.ax_box.riter;
}

static void one_free(ax_one *one)
{
	ax_pwarn("%s object is not allowed to free by user", one_name(one));
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(4, ui_model);
}

static ax_dump *any_dump(const ax_any *any)
{
	ui_model_cr self = AX_R_INIT(ax_any, any);
	ax_dump *blk = ax_dump_block(one_name(self.ax_one), ax_box_size(self.ax_box));

	int i = 0;
	ax_box_cforeach(self.ax_box, const ui_model_record *, row) {
		ui_model_record *r = (ui_model_record *)row;
		ax_dump_bind(blk, i, ax_trait_dump(ax_class_data(self.ax_box).elem_tr, r));
		i++;
	}
	return blk;
}

static ax_any *any_copy(const ax_any *any)
{
	return NULL;
}

static size_t box_size(const ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ui_model_cr self = AX_R_INIT(ax_box, box);
	return ax_obj_do0(self.ui_model->rc_tab.ax_box, size);
}

static size_t box_maxsize(const ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ui_model_cr self = AX_R_INIT(ax_box, box);
	return ax_obj_do0(self.ui_model->rc_tab.ax_box, maxsize);
}

static ax_iter box_begin(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ui_model_r self = AX_R_INIT(ax_box, box);
	ax_iter it = ax_obj_do0(self.ui_model->rc_tab.ax_box, begin);
	it.owner = self.ui_model;
	it.tr = &ui_model_tr.ax_box.iter;
	return it;
}

static ax_iter box_end(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ui_model_r self = AX_R_INIT(ax_box, box);
	ax_iter it = ax_obj_do0(self.ui_model->rc_tab.ax_box, end);
	it.owner = self.ui_model;
	it.tr = &ui_model_tr.ax_box.iter;
	return it;
}

static ax_iter box_rbegin(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ui_model_r self = AX_R_INIT(ax_box, box);
	ax_iter it = ax_obj_do0(self.ui_model->rc_tab.ax_box, rbegin);
	it.owner = self.ui_model;
	it.tr = &ui_model_tr.ax_box.riter;
	return it;

}

static ax_iter box_rend(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ui_model_r self = AX_R_INIT(ax_box, box);
	ax_iter it = ax_obj_do0(self.ui_model->rc_tab.ax_box, rend);
	it.owner = self.ui_model;
	it.tr = &ui_model_tr.ax_box.riter;
	return it;
}

static void box_clear(ax_box *box)
{
	CHECK_PARAM_NULL(box);
	ui_model_r self = AX_R_INIT(ax_box, box);
	size_t size = box_size(self.ax_box);
	ax_obj_do0(self.ui_model->rc_tab.ax_box, clear);
	for (int i = 0; i < size; i++)
		uiTableModelRowDeleted(self.ui_model->uimodel, i);
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	ui_model_r self = AX_R_INIT(ax_seq, seq);
	it->owner = self.ui_model->rc_tab.ax_vector;
	const ax_iter_trait *orig_tr = it->tr;
	it->tr = &self.ui_model->rc_tab.ax_box->tr->iter;
	size_t offset = ((uint8_t *)it->point - (uint8_t *)ax_vector_buffer(self.ui_model->rc_tab.ax_vector)) / self.ax_box->env.elem_tr->t_size;
	ax_fail fail = ax_obj_do(self.ui_model->rc_tab.ax_seq, insert, it, val, ap);
	if (fail)
		goto out;
	uiTableModelRowInserted(self.ui_model->uimodel, offset);
out:
	it->owner = seq;
	it->tr = orig_tr;
	return fail;
}

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	ui_model_r self = AX_R_INIT(ax_seq, seq);
	ax_fail fail = ax_obj_do(self.ui_model->rc_tab.ax_seq, push, val, ap);
	if (fail)
		return fail;
	uiTableModelRowInserted(self.ui_model->uimodel, box_size(self.ax_box) - 1);
	return false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ui_model_r self = AX_R_INIT(ax_seq, seq);
	(void)ax_obj_do0(self.ui_model->rc_tab.ax_seq, pop);
	uiTableModelRowDeleted(self.ui_model->uimodel, box_size(self.ax_box));
	return false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ui_model_r self = AX_R_INIT(ax_seq, seq);
	ax_obj_do0(self.ui_model->rc_tab.ax_seq, invert);
	for (int i = 0; i < box_size(self.ax_box); i++)
		uiTableModelRowChanged(self.ui_model->uimodel, i);
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(ax_r(ax_seq, seq).ax_box));
	ui_model_r self = AX_R_INIT(ax_seq, seq);
	size_t old_size = box_size(self.ax_box);
	ax_fail fail = ax_obj_do(self.ui_model->rc_tab.ax_seq, trunc, size);
	if (fail)
		return true;

	if (size > old_size)
		for (int i = old_size; i < size; i++)
			uiTableModelRowInserted(self.ui_model->uimodel, i);
	else
		for (int i = size; i < old_size; i++)
			uiTableModelRowDeleted(self.ui_model->uimodel, i);
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(ax_cr(ax_seq, seq).ax_box));
	ui_model_cr self = AX_R_INIT(ax_seq, seq);
	ax_iter it = ax_obj_do(self.ui_model->rc_tab.ax_seq, at, index);
	it.owner = (void *)self.ui_model;
	it.tr = &ui_model_tr.ax_box.iter;
	return it;
}

static void *seq_last(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ui_model_cr self = AX_R_INIT(ax_seq, seq);
	return ax_obj_do0(self.ui_model->rc_tab.ax_seq, last);
}

static void *seq_first(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);
	ui_model_cr self = AX_R_INIT(ax_seq, seq);
	return ax_obj_do0(self.ui_model->rc_tab.ax_seq, first);
}

const ax_seq_trait ui_model_tr =
{
	.ax_box = {
		.ax_any = {
			.ax_one = {
				.free = one_free,
				.name = one_name,
			},
			.dump = any_dump,
			.copy = any_copy,
		},
		.iter = {
			.norm = true,
			.type = AX_IT_RAND,
			.move = citer_move,
			.next = citer_next,
			.prev = citer_prev,
			.less = citer_less,
			.dist = citer_dist,
			.box  = citer_box,
			.get = citer_get,
			.set = iter_set,
			.erase = iter_erase,
		},
		.riter = {
			.norm = false,
			.type = AX_IT_RAND,
			.move = rciter_move,
			.next = rciter_next,
			.prev = rciter_prev,
			.dist = rciter_dist,
			.less = citer_less,
			.get = citer_get,
			.set = iter_set,
			.erase = riter_erase,
		},

		.size = box_size,
		.maxsize = box_maxsize,

		.begin = box_begin,
		.end = box_end,
		.rbegin = box_rbegin,
		.rend = box_rend,

		.clear = box_clear,

	},
	.push = seq_push,
	.pop = seq_pop,
	.pushf = NULL,
	.popf = NULL,
	.invert = seq_invert,
	.trunc = seq_trunc,
	.at = seq_at,
	.last = seq_last,
	.first = seq_first,
	.insert = seq_insert,
};

static ax_dump *record_dump(const void* p)
{
	const ui_model_record *r = p;
	if (!r->m)
		return ax_dump_symbol("pending_record");

	ax_dump *dmp = ax_dump_block("record", r->m->column_count);
	for (int i = 0; i < r->m->column_count; i++) {
		field_union field = { ui_model_record_at((ui_model_record *)r, r->m, i) };
		ax_dump *item;

		switch (r->m->type_tab[i]) {
			case UI_MODEL_TEXT:
				item = ax_dump_block("text", 1);
				ax_dump_bind(item, 0, ax_dump_str(field.text->text));
				break;
			case UI_MODEL_BUTTON:
				item = ax_dump_block("button", 1);
				ax_dump_bind(item, 0, ax_dump_str(field.button->text));
				break;
			case UI_MODEL_CHECKBOX:
				item = ax_dump_block("checkbox", 1);
				ax_dump_bind(item, 0, ax_dump_symbol(field.checkbox->checked ? "checked" : "unchecked"));
				break;
			case UI_MODEL_IMAGE:
				item = ax_dump_block("image", 1);
				ax_dump_bind(item, 0, ax_dump_ptr(field.image->image));
				break;
			case UI_MODEL_PROGRESS:
				item = ax_dump_block("progress", 1);
				ax_dump_bind(item, 0, ax_dump_uint(field.progress->progress));
				break;
			case UI_MODEL_CHECKBOX_TEXT:
				item = ax_dump_block("checkbox_text", 2);
				ax_dump_bind(item, 0, ax_dump_symbol(field.checkbox_text->checked ? "checked" : "unchecked"));
				ax_dump_bind(item, 1, ax_dump_str(field.checkbox_text->text));
				break;
			case UI_MODEL_IMAGE_TEXT:
				item = ax_dump_block("image_text", 2);
				ax_dump_bind(item, 0, ax_dump_ptr(field.image->image));
				ax_dump_bind(item, 1, ax_dump_str(field.checkbox_text->text));
				break;
		}
		ax_dump_bind(dmp, i, item);
	}
	return dmp;
}

static bool record_equal(const void* a, const void *b)
{
	return false;
}

static bool record_less(const void* a, const void *b)
{
	return false;
}

static void record_free(void* p)
{
	ui_model_record *r = *(void **)p;
	ui_model_record_free(r, r->m);
}

static ax_fail record_copy(void* a, const void *b)
{
	ui_model_record *r = *(void **)b;
	*(void **)a = r;
	return false;
}

static const ax_trait t_record = {
        .t_size  = sizeof(void*),
        .t_equal = record_equal,
        .t_less  = record_less,
        .t_dump  = record_dump,
        .t_free  = record_free,
        .t_copy  = record_copy,
        .t_link  = true
};

static int NumColumns(uiTableModelHandler *mh, uiTableModel *m)
{
        ui_model_r model = { HANDLER_MODEL(mh) };
        return ax_box_size(model.ax_box);
}

static uiTableValueType ColumnType(uiTableModelHandler *mh, uiTableModel *m, int column)
{
        ui_model_r model = { HANDLER_MODEL(mh) };
	int field_idx = column / 4;
	int who = column % 4;

	return column_type(model.ui_model->type_tab[field_idx], who);
}

static int NumRows(uiTableModelHandler *mh, uiTableModel *m)
{
	ui_model_r model = { HANDLER_MODEL(mh) };
	return ax_box_size(model.ax_box);
}

static uiTableValue *CellValue(uiTableModelHandler *mh, uiTableModel *m, int row, int column)
{
	ui_model_r model = { HANDLER_MODEL(mh) };
	int field_idx = column / 4;
	int field_elem = column % 4;
	ax_iter it = seq_at(model.ax_seq, row);
	ui_model_record *record = ax_iter_get(&it);
	void *field = (void *)(record->data + model.ui_model->column_off_tab[field_idx]);

	switch (model.ui_model->type_tab[field_idx]) {
		case UI_MODEL_TEXT:
			return column_text_get(field, field_elem);
		case UI_MODEL_CHECKBOX:
			return column_checkbox_get(field, field_elem);
		case UI_MODEL_BUTTON:
			return column_button_get(field, field_elem);
		case UI_MODEL_PROGRESS:
			return column_progress_get(field, field_elem);
		case UI_MODEL_IMAGE:
			return column_image_get(field, field_elem);
		case UI_MODEL_CHECKBOX_TEXT:
			return column_checkbox_text_get(field, field_elem);
		case UI_MODEL_IMAGE_TEXT:
			return column_image_text_get(field, field_elem);
		default:
			return NULL;
	}
}

static void SetCellValue(uiTableModelHandler *mh, uiTableModel *uimodel, int row, int column, const uiTableValue *v)
{
        ui_model_r model = { HANDLER_MODEL(mh) };
	ui_model *m = model.ui_model;

	int field_idx = column / 4;
	int field_elem = column % 4;
	ax_iter it = seq_at(model.ax_seq, row);
	ui_model_record *record = ax_iter_get(&it);
	field_union field = { (void *)(record->data + model.ui_model->column_off_tab[field_idx]) };

	switch (model.ui_model->type_tab[field_idx]) {
		case UI_MODEL_TEXT:
			if (m->on_changed && !m->on_changed(m, field_idx, row, uiTableValueString(v), m->data_on_changed))
				break;
			column_text_set(field.text, field_elem, v);
			break;
		case UI_MODEL_CHECKBOX:
			if (m->on_checked && !m->on_checked(m, field_idx, row, uiTableValueInt(v), m->data_on_changed))
				break;
			column_checkbox_set(field.checkbox, field_elem, v);
			break;
		case UI_MODEL_BUTTON:
			if (m->on_clicked)
				m->on_clicked(m, field_idx, row, m->data_on_changed);
			column_button_set(field.button, field_elem, v);
			break;
		case UI_MODEL_PROGRESS:
			column_progress_set(field.progress, field_elem, v);
			break;
		case UI_MODEL_IMAGE:
			column_image_set(field.image, field_elem, v);
			break;
		case UI_MODEL_CHECKBOX_TEXT:
			if (field_elem == 0 && m->on_checked && !m->on_checked(m, field_idx, row, uiTableValueInt(v), m->data_on_changed))
				break;
			if (field_elem == 2 && m->on_changed && !m->on_changed(m, field_idx, row, uiTableValueString(v), m->data_on_changed))
				break;
			column_checkbox_text_set(field.checkbox_text, field_elem, v);
			break;
		case UI_MODEL_IMAGE_TEXT:
			if (m->on_changed && !m->on_changed(m, field_idx, row, uiTableValueString(v), m->data_on_changed))
				break;
			column_image_text_set(field.image_text, field_elem, v);
			break;
	}
}


ax_concrete_creator(ui_model, int types[], size_t count)
{
	ax_vector_r rc_tab= AX_R_NULL;
	ax_seq *seq = NULL;
	uiTableModelHandler *handler = NULL;
	uiTableModel *uimodel = NULL;
	size_t *offset_tab = NULL;
	char *type_tab = NULL;

	offset_tab = malloc(count * sizeof *offset_tab);
	if (!offset_tab)
		goto fail;

	size_t off = 0;
	offset_tab[0] = 0;
	for (int i = 1; i < count; i++) {
		off += column_size(types[i - 1]);
		offset_tab[i] = off;
	}

	type_tab = malloc(count * sizeof *type_tab);
	if (!type_tab)
		goto fail;
	for (int i = 0; i < count; i++)
		type_tab[i] = types[i];

	seq = malloc(sizeof(ui_model));
	if (!seq)
		goto fail;

	handler = malloc(sizeof(uiTableModelHandler) + sizeof(void *));
	if (!handler)
		goto fail;

        handler->CellValue = CellValue,
        handler->ColumnType = ColumnType,
        handler->NumColumns = NumColumns,
        handler->NumRows = NumRows,
        handler->SetCellValue = SetCellValue,
	HANDLER_MODEL(handler) = (ui_model *)seq;

	uimodel = uiNewTableModel(handler);
	if (!uimodel)
		goto fail;

	rc_tab = ax_new(ax_vector, &t_record);;
	if (!rc_tab.ax_one)
		goto fail;

	ui_model model_init = {
		.ax_seq = {
			.tr = &ui_model_tr,
			.env.ax_box.elem_tr = &t_record,
		},
		.rc_tab = rc_tab,
		.uimodel = uimodel,
		.handler = handler,
		.column_count = count,
		.column_off_tab = offset_tab,
		.type_tab = type_tab,
	};

	memcpy(seq, &model_init, sizeof model_init);
	return seq;
fail:
	free(seq);
	uiFreeTableModel(uimodel);
	free(handler);
	free(types);
	free(offset_tab);
	ax_one_free(rc_tab.ax_one);
	return NULL;
}

ui_model_record *ui_model_record_alloc(ui_model *m)
{
	ui_model_record *r = NULL;
	size_t size = m->column_off_tab[m->column_count - 1] + column_size(m->type_tab[m->column_count - 1]);
	r = malloc(sizeof(ui_model_record) + size);
	if (!r)
		return NULL;
	memset(r, 0, sizeof(ui_model_record) + size);
	r->m = m;
	return r;
}

void *ui_model_record_at(ui_model_record *r, const ui_model *m, int column)
{
	ax_assert(column < m->column_count, "column is too big");
	return r->data + m->column_off_tab[column];
}

size_t ui_model_column_count(const ui_model *m)
{
	return m->column_count;
}

void *ui_model_raw_model(ui_model *m)
{
	return m->uimodel;
}

static void free_field(void *field_ptr, int type)
{
	field_union field = { field_ptr };

	switch (type) {
		case UI_MODEL_TEXT:
			free(field.text->text);
			break;
		case UI_MODEL_BUTTON:
			free(field.button->text);
			break;
		case UI_MODEL_IMAGE:
			ax_one_free((ax_one *)field.image->image);
			break;
		case UI_MODEL_CHECKBOX_TEXT:
			free(field.checkbox_text->text);
			break;
		case UI_MODEL_IMAGE_TEXT:
			ax_one_free((ax_one *)field.image_text->image);
			free(field.image_text->text);
			break;
	}
}

void ui_model_record_free(ui_model_record *r, const ui_model *m)
{
	ui_model_r self = AX_R_INIT(ui_model, r->m);
	for (int i = 0; i < self.ui_model->column_count; i++)
		free_field((void *)(r->data + self.ui_model->column_off_tab[i]), self.ui_model->type_tab[i]);
}

void __ui_model_free(ui_model *m)
{
	if (!m)
		return;
	ui_model_r self = AX_R_INIT(ui_model, m);

	ax_iter it = ax_box_rbegin(self.ax_box);
	ax_iter end = ax_box_rend(self.ax_box);
	while (!ax_iter_equal(&it, &end)) {
		ax_iter_erase(&it);
		end = ax_box_rend(self.ax_box);
	}

	uiFreeTableModel(self.ui_model->uimodel);
	free(self.ui_model->handler);
	free(self.ui_model->type_tab);
	free(self.ui_model->column_off_tab);
	ax_one_free(self.ui_model->rc_tab.ax_one);
	free(m);
}


void ui_model_on_clicked(ui_model *m, void (*f)(ui_model *sender, int column, int row, void *arg), void *arg)
{
	m->on_clicked = f;
	m->data_on_clicked = arg;
}

void ui_model_on_checked(ui_model *m, bool (*f)(ui_model *sender, int column, int row, bool new_checked, void *arg), void *arg)
{
	m->on_checked = f;
	m->data_on_checked = arg;
}

void ui_model_on_changed(ui_model *m, bool (*f)(ui_model *sender, int column, int row, const char *new_string, void *arg), void *arg)
{
	m->on_changed = f;
	m->data_on_changed = arg;
}

void ui_model_update(ui_model *m, int index)
{
	uiTableModelRowChanged(m->uimodel, index);
}
