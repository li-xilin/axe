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

#include <ax/queue.h>
#include <ax/deq.h>
#include <ax/tube.h>
#include <ax/any.h>
#include <ax/mem.h>
#include <ax/dump.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "check.h"

#undef free

struct ax_queue_st
{
	ax_tube tube;
	ax_deq_r deq;
};

static ax_fail  tube_push(ax_tube *tube, const void *val, va_list *ap);
static void     tube_pop(ax_tube *tube);
static size_t   tube_size(const ax_tube *tube);
static void    *tube_prime(const ax_tube *tube);
static ax_any  *any_copy(const ax_any *any);
static ax_dump *any_dump(const ax_any *any);
static void     one_free(ax_one *one);

const ax_tube_trait ax_queue_tr =
{
		.any = {
			.one = {
				.name = AX_QUEUE_NAME,
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

	ax_queue_cr self_r = { .tube = tube };
	return ax_deq_tr.push(self_r.queue->deq.seq, val, ap);
}

static void tube_pop(ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_queue_cr self_r = { .tube = tube };
	ax_deq_tr.popf(self_r.queue->deq.seq);
}

static size_t tube_size(const ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_queue_cr self_r = { .tube = tube };
	return ax_deq_tr.box.size(self_r.queue->deq.box);
}

static void *tube_prime(const ax_tube *tube)
{
	CHECK_PARAM_NULL(tube);

	ax_queue_cr self_r = { .tube = tube };
	return ax_deq_tr.first(self_r.queue->deq.seq);
}

static ax_any *any_copy(const ax_any *any)
{
	return NULL;
}

static ax_dump *any_dump(const ax_any *any)
{
	ax_queue_cr self = { .any = any };
	ax_dump *dmp = ax_any_dump(self.queue->deq.any);
	ax_dump_set_name(dmp, ax_one_name(self.one));
	return dmp;
}

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_queue_r self_r = { .one = one };
	ax_deq_tr.box.any.one.free(self_r.queue->deq.one);
	free(one);
}

ax_tube *__ax_queue_construct(const ax_trait *elem_tr)
{
	CHECK_PARAM_NULL(elem_tr);

	ax_tube *self= NULL;
	ax_deq_r deq = ax_class_new(deq, elem_tr);
	if (!deq.one)
		goto fail;

	self = malloc(sizeof(ax_queue));
	if (!self) {
		goto fail;
	}

	ax_queue queue_init = {
		.tube = {
			.tr = &ax_queue_tr,
			.env.elem_tr = elem_tr,
		},
		.deq = deq,
	};

	memcpy(self, &queue_init, sizeof queue_init);
	return self;
fail:
	ax_one_free(deq.one);
	free(self);
	return NULL;
}

