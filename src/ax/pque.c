/*
 * Copyright (c) 2022 Li hsilin <lihsilyn@gmail.com>
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

#include "ax/pque.h"
#include "ax/class.h"
#include "ax/tube.h"
#include "ax/any.h"
#include "ax/mem.h"
#include "ax/dump.h"
#include "ax/static/heap.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "check.h"

#undef free

ax_begin_entry(pque)
	struct ax_heap_st heap;
ax_end;

static ax_fail  tube_push(ax_tube *tube, const void *val, va_list *ap);
static void     tube_pop(ax_tube *tube);
static size_t   tube_size(const ax_tube *tube);
static const void *tube_prime(const ax_tube *tube);
static ax_any  *any_copy(const ax_any *any);
static ax_dump *any_dump(const ax_any *any);
static void     one_free(ax_one *one);
static const char *one_name(const ax_one *one);

const ax_tube_trait ax_pque_tr =
{
		.any = {
			.one = {
				.name = one_name,
				.free = one_free,
			},
			.copy = any_copy,
			.dump = any_dump,
		},
		.push = tube_push,
		.pop = tube_pop,
		.size = tube_size,
		.prime = tube_prime,
};


static ax_fail tube_push(ax_tube *tube, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(tube);

	ax_pque_r self = { .tube = tube };
	return ax_heap_insert(&self.pque->heap, ax_trait_in(self.tube->env.elem_tr, val));
}

static void tube_pop(ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_pque_r self = { .tube = tube };
	ax_heap_pop(&self.pque->heap);
}

static size_t tube_size(const ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_pque_cr self = { .tube = tube };
	return ax_heap_size(&self.pque->heap);
}

static const void *tube_prime(const ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_pque_cr self = { .tube = tube };
	return ax_heap_top(&self.pque->heap);
}

static ax_any *any_copy(const ax_any *any)
{
	return NULL;
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_pque_cr self = { .any = any };

	if (ax_heap_size(&self.pque->heap) == 0)
		return ax_dump_block(ax_one_name(self.one), 0);

	ax_dump *dmp = ax_dump_block(ax_one_name(self.one), 1);
	ax_dump *top = self.tube->env.elem_tr->dump(ax_heap_top(&self.pque->heap),
			self.tube->env.elem_tr->size);
	ax_dump_pair(ax_dump_symbol("TOP"), top);
	return dmp;
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_pque_r self = { .one = one };
	ax_heap_destroy(&self.pque->heap);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(3, pque);
}

static bool less_then(const void *a, const void *b, void *ctx)
{
	const ax_trait *etr = (const void *)ctx;
	return etr->less(a, b, etr->size);
}


ax_class_constructor(pque, const ax_trait *tr)
{
	CHECK_PARAM_NULL(tr);

	ax_tube *self= NULL;

	self = malloc(sizeof(ax_pque));
	if (!self) {
		goto fail;
	}

	ax_pque pque_init = {
		.tube = {
			.tr = &ax_pque_tr,
			.env.elem_tr = tr,
		},
	};

	if (ax_heap_init(&pque_init.heap, tr->size, 1, less_then, (void *)tr))
		goto fail;

	memcpy(self, &pque_init, sizeof pque_init);
	return self;
fail:
	free(self);
	return NULL;
}

