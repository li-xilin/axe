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


#include <axe/seq.h>
#include <axe/vail.h>
#include <axe/scope.h>
#include <axe/base.h>
#include <axe/error.h>

#include "check.h"

ax_seq *ax_seq_vinit(ax_scope *scope, ax_seq_construct_f *builder, const char *fmt, va_list varg) 
{
	CHECK_PARAM_NULL(builder);
	CHECK_PARAM_NULL(fmt);

	ax_base *base = ax_one_base(ax_r(scope, scope).one);

	ax_vail *vail = NULL;
	ax_seq *seq = NULL;

	vail = ax_vail_vcreate(base, fmt, varg);
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

	seq = builder(base, ax_stuff_traits(elem_type));
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

	ax_base *base = ax_one_base(ax_r(seq, seq).one);

	ax_vail *vail = NULL;
	vail = ax_vail_vcreate(base, fmt, varg);

	if (!vail) {
		return ax_true;
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
	return ax_false;
fail:
	for (int i = 0; i < npush; i++) {
		ax_seq_pop(seq);
	}
	ax_vail_destroy(vail);
	return ax_true;
}

ax_fail ax_seq_pushl(ax_seq *seq, const char *fmt, ...)
{
	va_list varg;
	va_start(varg, fmt);
	ax_fail fail = ax_seq_vpushl(seq, fmt, varg);
	va_end(varg);
	return fail;
}

#if 0
ax_fail ax_seq_vmpop(ax_seq *seq, unsigned int count, va_list varg)
{
	CHECK_PARAM_NULL(seq);
	ax_base *base = ax_one_base(ax_r(seq, seq).one);
	ax_pool *pool = ax_base_pool(base);
	const ax_stuff_trait *etr = ax_box_elem_tr(ax_r(seq, seq).box);
	
	int i;
	for (i = 0; i < count; i++) {
		void *ptr = va_arg(varg, void *);
		void *val = ax_seq_last(seq);
		void *pval = etr->link
			? &val
			: val;
		if (etr->link)
			
		if (etr->copy(pool, ptr, pval, etr->size)) {
			ax_base_set_errno(base, AX_ERR_NOMEM);
			goto fail;
		}
		ax_seq_pop(seq);
	}
fail:
	for (int j = 0; j < i; j++) {
	}
	
}

ax_fail ax_seq_mpop(ax_seq *seq, unsigned int count, ...)
{
	CHECK_PARAM_NULL(seq);
}

#endif
