/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
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

#include "ax/queue.h"
#include "ax/deq.h"
#include "ax/mem.h"
#include "ax/dump.h"
#include "check.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#undef free

ax_concrete_begin(ax_queue)
	ax_deq_r deq;
ax_end;

static ax_fail  tube_push(ax_tube *tube, const void *val, va_list *ap);
static void     tube_pop(ax_tube *tube);
static size_t   tube_size(const ax_tube *tube);
static const void *tube_prime(const ax_tube *tube);
static ax_any  *any_copy(const ax_any *any);
static ax_dump *any_dump(const ax_any *any);
static void     one_free(ax_one *one);
static const char *one_name(const ax_one *one);

const ax_tube_trait ax_queue_tr =
{
		.ax_any = {
			.ax_one = {
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

	ax_queue_r self = AX_R_INIT(ax_tube, tube);
	return ax_deq_tr.push(self.ax_queue->deq.ax_seq, val, ap);
}

static void tube_pop(ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_queue_cr self = AX_R_INIT(ax_tube, tube);
	ax_deq_tr.popf(self.ax_queue->deq.ax_seq);
}

static size_t tube_size(const ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_queue_cr self = AX_R_INIT(ax_tube, tube);
	return ax_deq_tr.ax_box.size(self.ax_queue->deq.ax_box);
}

static const void *tube_prime(const ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_queue_cr self = AX_R_INIT(ax_tube, tube);
	return ax_deq_tr.first(self.ax_queue->deq.ax_seq);
}

static ax_any *any_copy(const ax_any *any)
{
	return NULL;
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_queue_cr self = AX_R_INIT(ax_any, any);
	ax_dump *dmp = ax_any_dump(self.ax_queue->deq.ax_any);
	ax_dump_set_name(dmp, ax_one_name(self.ax_one));
	return dmp;
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_queue_r self = AX_R_INIT(ax_one, one);
	ax_deq_tr.ax_box.ax_any.ax_one.free(self.ax_queue->deq.ax_one);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(3, ax_queue);
}

ax_tube *__ax_queue_construct(const ax_trait *elem_tr)
{
	CHECK_PARAM_NULL(elem_tr);

	ax_tube *self= NULL;
	ax_deq_r deq = AX_R_NULL;

	deq = ax_new(ax_deq, elem_tr);
	if (!deq.ax_one)
		goto fail;

	self = malloc(sizeof(ax_queue));
	if (!self) {
		goto fail;
	}

	ax_queue queue_init = {
		.ax_tube = {
			.tr = &ax_queue_tr,
			.env.elem_tr = elem_tr,
		},
		.deq = deq,
	};

	memcpy(self, &queue_init, sizeof queue_init);
	return self;
fail:
	ax_one_free(deq.ax_one);
	free(self);
	return NULL;
}

