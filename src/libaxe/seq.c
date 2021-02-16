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

	ax_base *base = ax_scope_base(scope);

	ax_vail *vail = ax_vail_vcreate(base, fmt, varg);

	if (vail == NULL) {
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

	ax_seq *seq = builder(base, ax_stuff_traits(elem_type));
	if (seq == NULL) {
		ax_vail_destroy(vail);
		return NULL;
	}

	ax_vail_get(vail, 0, &vinfo);
	int type = vinfo.type;

	for (int i = 0; i < vail_size; i++) {
		ax_vail_get(vail, i, &vinfo);
		for (int j = 0; j < vinfo.size; j++)
			ax_seq_push(seq, vinfo.ptr + j * ax_stuff_size(type));
	}
	ax_vail_destroy(vail);
	ax_scope_attach(scope, (ax_one *)seq);
	return seq;
}


ax_seq *ax_seq_init(ax_scope *scope, ax_seq_construct_f *builder, const char *fmt, ...) 
{
	va_list varg;
	va_start(varg, fmt);
	ax_seq *seq = ax_seq_vinit(scope, builder, fmt, varg);
	va_end(varg);
	return seq;
}
