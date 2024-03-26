/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
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

#include "ut/case.h"
#include <ax/mem.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct message_st
{
	ax_link link;
	const char *file;
	int line;
	char text[];
};

static void case_free(void *p);
static bool case_less(const void *p1, const void *p2);
static bool case_equal(const void *p1, const void *p2);
static ax_fail case_copy(void* dst, const void* src);

const ax_trait ut_case_tr =
{
	.t_copy = case_copy,
	.t_less = case_less,
	.t_equal = case_equal,
	.t_free = case_free,
	.t_size = sizeof(ut_case),
	.t_link = false
};

static void case_free(void *p)
{
	ut_case *tc = p;
	while (!ax_link_is_empty(&tc->msg_list)) {
		struct message_st *m = ax_link_first_entry(&tc->msg_list, struct message_st, link);
		ax_link_del(&m->link);
		free(m);
	}
	free(tc->name);
	free(tc->file);
	free(tc->log);
}

static bool case_less(const void *p1, const void *p2)
{
	const ut_case *tc1 = p1;
	const ut_case *tc2 = p2;
	return tc1->priority < tc2->priority;
}

static bool case_equal(const void *p1, const void *p2)
{

	const ut_case *tc1 = p1;
	const ut_case *tc2 = p2;
	return tc1->priority == tc2->priority;
}

static ax_fail case_copy(void* dst, const void* src)
{
	const ut_case *src_tc = src;
	ut_case *dst_tc = dst;
	memcpy(dst_tc, src_tc, sizeof *dst_tc);
	dst_tc->name = dst_tc->log = dst_tc->file = NULL;
	dst_tc->name = ax_strdup(src_tc->name);

	if (!src_tc->name) {
		errno = EINVAL;
		goto fail;
	}

	if (src_tc->log)
		if (!(dst_tc->log =  ax_strdup(src_tc->log)))
			goto fail;
	if (src_tc->file)
		if (!(dst_tc->file =  ax_strdup(src_tc->file)))
			goto fail;
	ax_link_init(&dst_tc->msg_list);

	return false;
fail:
	free(dst_tc->name);
	free(dst_tc->log);
	free(dst_tc->file);
	return true;
}

ax_fail ut_case_add_text(ut_case *c, const char *file, int line, char *text)
{
	int size = strlen(text) + 1;
	struct message_st *m = malloc(sizeof *m + size);
	if (!m)
		return true;
	memcpy(m->text, text, size);
	m->file = file;
	m->line = line;
	ax_link_add_back(&m->link, &c->msg_list);
	return false;
}

void ut_case_enum_text(ut_case *c, ax_case_enum_text_f *f, void *ctx)
{
	ax_link *link;
	ax_link_foreach(link, &c->msg_list) {
		struct message_st *m = ax_link_entry(link, struct message_st, link);
		f(c,  m->file, m->line, m->text, ctx);
	}
}

