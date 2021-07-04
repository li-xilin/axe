/*
 * Copyright (c) 2020 - 2021 Li hsilin <lihsilyn@gmail.com>
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


#include <ax/seq.h>
#include <ax/vail.h>
#include <ax/scope.h>
#include <ax/dump.h>

#include "check.h"

#include <errno.h>

ax_seq *ax_seq_vinit(ax_scope *scope, ax_seq_construct_f *builder, const char *fmt, va_list varg) 
{
	CHECK_PARAM_NULL(builder);
	CHECK_PARAM_NULL(fmt);

	ax_vail *vail = NULL;
	ax_seq *seq = NULL;

	vail = ax_vail_vcreate(fmt, varg);
	if (!vail) {
		return NULL;
	}
	int vail_size = ax_vail_size(vail);
	ax_assert(vail_size != 0, "initial list is empty");

	ax_vail_info vinfo;
	ax_vail_get(vail, 0, &vinfo);
	int elem_type = vinfo.type;

	size_t elem_count = 0;
	for (int i = 1; i < vail_size; i++) {
		ax_vail_get(vail, i, &vinfo);
		ax_assert(vinfo.type == elem_type, "different types in initial list");
		elem_count += vinfo.size;
	}

	seq = builder(ax_stuff_traits(elem_type));
	if (!seq) {
		ax_vail_destroy(vail);
		return NULL;
	}

	ax_vail_get(vail, 0, &vinfo);
	int type = vinfo.type;

	for (int i = 0; i < vail_size; i++) {
		ax_vail_get(vail, i, &vinfo);
		for (int j = 0; j < vinfo.size; j++)
			if (ax_seq_push(seq, (ax_byte*)vinfo.ptr + j * ax_stuff_size(type)))
				goto fail;

	}
	ax_vail_destroy(vail);
	ax_scope_attach(scope, (ax_one *)seq);
	return seq;
fail:
	ax_one_free(ax_r(seq, seq).one);
	ax_vail_destroy(vail);
	return NULL;
}

ax_seq *ax_seq_init(ax_scope *scope, ax_seq_construct_f *builder, const char *fmt, ...) 
{
	va_list varg;
	va_start(varg, fmt);
	ax_seq *seq = ax_seq_vinit(scope, builder, fmt, varg);
	va_end(varg);
	return seq;
}

ax_fail ax_seq_vpushl(ax_seq *seq, const char *fmt, va_list varg)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(fmt);

	ax_vail *vail = NULL;
	vail = ax_vail_vcreate(fmt, varg);

	if (!vail) {
		return true;
	}
	int vail_size = ax_vail_size(vail);
	ax_assert(vail_size != 0, "initial list is empty");

	ax_vail_info vinfo;
	ax_vail_get(vail, 0, &vinfo);
	int elem_type = vinfo.type;
	(void)elem_type;

	size_t elem_count = 0;
	for (int i = 1; i < vail_size; i++) {
		ax_vail_get(vail, i, &vinfo);
		ax_assert(vinfo.type == elem_type, "different types in initial list");
		elem_count += vinfo.size;
	}

	ax_vail_get(vail, 0, &vinfo);
	int type = vinfo.type;

	const ax_stuff_trait *etr = ax_stuff_traits(type);
	ax_assert(etr->size == ax_box_elem_tr(ax_r(seq, seq).box)->size, "data type in initial list is invalid");

	int npush = 0;
	for (int i = 0; i < vail_size; i++) {
		ax_vail_get(vail, i, &vinfo);
		for (int j = 0; j < vinfo.size; j++)
			if (ax_seq_push(seq, (ax_byte*)vinfo.ptr + j * etr->size))
				goto fail;
		npush ++;
	}
	ax_vail_destroy(vail);
	return false;
fail:
	for (int i = 0; i < npush; i++) {
		ax_seq_pop(seq);
	}
	ax_vail_destroy(vail);
	return true;
}

ax_fail ax_seq_pushl(ax_seq *seq, const char *fmt, ...)
{
	va_list varg;
	va_start(varg, fmt);
	ax_fail fail = ax_seq_vpushl(seq, fmt, varg);
	va_end(varg);
	return fail;
}

size_t ax_seq_array(ax_seq *seq, void *elems[], size_t len)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(elems);
	
	ax_iter pos = ax_box_rbegin(ax_r(seq, seq).box),
		end = ax_box_rend(ax_r(seq, seq).box);

	size_t i;
	for (i = 0; i < len && !ax_iter_equal(&pos, &end); i++) {
		elems[i] = ax_iter_get(&pos);
		ax_iter_next(&pos);
	}
	return i; 
}

ax_dump *ax_seq_dump(const ax_seq *seq)
{
	ax_seq_cr self = ax_cr(seq, seq);
	size_t size = ax_box_size(self.box);
	ax_dump *block_dmp = NULL;

	block_dmp = ax_dump_block(ax_one_name(self.one), size);
	if (!block_dmp)
		return NULL;

	const ax_stuff_trait *etr = ax_box_elem_tr(self.box);

	size_t i = 0;
	ax_dump *elem_dmp;
	if (etr->link) {
		ax_box_cforeach(self.box, const void *, p) {
			elem_dmp = etr->dump(&p, etr->size);
			if (!elem_dmp)
				goto fail;
			ax_dump_bind(block_dmp, i, elem_dmp);
			i++;
		}
	} else {
		ax_box_cforeach(self.box, const void *, p) {
			elem_dmp = etr->dump(p, etr->size);
			if (!elem_dmp)
				goto fail;
			ax_dump_bind(block_dmp, i, elem_dmp);
			i++;
		}
	}
	return block_dmp;
fail:
	ax_dump_free(block_dmp);
	return NULL;
}
