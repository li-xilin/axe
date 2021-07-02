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

#include <ax/obj.h>
#include <ax/stuff.h>
#include <ax/mem.h>
#include <ax/list.h>
#include <ax/vector.h>
#include <ax/avl.h>
#include <ax/hmap.h>
#include <ax/queue.h>
#include <ax/stack.h>
#include <ax/btrie.h>

#include <string.h>
#include <errno.h>

enum {
	OBJ_VECTOR,
	OBJ_LIST,
	OBJ_AVL,
	OBJ_HMAP,
	OBJ_QUEUE,
	OBJ_STACK,
	OBJ_BTRIE
};

int str2obj(const char *s)
{
	if (strcmp(s, "ax_vector") == 0)
		return OBJ_VECTOR;
	else if (strcmp(s, "ax_list") == 0)
		return OBJ_LIST;
	else if (strcmp(s, "ax_avl") == 0)
		return OBJ_AVL;
	else if (strcmp(s, "ax_hmap") == 0)
		return OBJ_HMAP;
	else if (strcmp(s, "ax_queue") == 0)
		return OBJ_QUEUE;
	else if (strcmp(s, "ax_stack") == 0)
		return OBJ_STACK;
	else if (strcmp(s, "ax_btrie") == 0)
		return OBJ_BTRIE;
	return -1;
}

ax_one *__ax_obj_construct(const char *type, const char *desc, ...)
{
	int siz = strlen(desc) + 1;
	char desc_buf[siz];
	memcpy(desc_buf, desc, siz);
	char *next = desc_buf, *p;
	va_list vl;
	const ax_stuff_trait *tr_tab[4];
	int tr_cnt = 0;

	va_start(vl, desc);
	while ((p = ax_strsplit(&next, ':'))) {
		int type = ax_stuff_stoi(p);
		ax_assert(type != -1, "unsupported trait `%s` in describe string", p);
		ax_assert(tr_cnt <= 4, "too many types in describe string");
		tr_tab[tr_cnt++] = (type == 0)
			? va_arg(vl, ax_stuff_trait *)
			:  ax_stuff_traits(type);
	}

	int obj = str2obj(type);
	ax_assert(obj != -1, "unsupported obj type `%s`", type);

#ifdef AX_DEBUG
	int tr_ex;
	switch(obj) {
		case OBJ_VECTOR:
			tr_ex = 1;
			break;
		case OBJ_LIST:
			tr_ex = 1;
			break;
		case OBJ_AVL:
			tr_ex = 2;
			break;
		case OBJ_HMAP:
			tr_ex = 2;
			break;
		case OBJ_QUEUE:
			tr_ex = 1;
			break;
		case OBJ_STACK:
			tr_ex = 1;
			break;
		case OBJ_BTRIE:
			tr_ex = 2;
			break;
	}
	ax_assert(tr_ex == tr_cnt, "object %s expect %d traits, but %d", type, tr_ex, tr_cnt);
#endif

	ax_one *inst;
	switch(obj) {
		case OBJ_VECTOR:
			inst = (ax_one *)__ax_vector_construct(tr_tab[0]);
			break;
		case OBJ_LIST:
			inst = (ax_one *)__ax_list_construct(tr_tab[0]);
			break;
		case OBJ_AVL:
			inst = (ax_one *)__ax_avl_construct(tr_tab[0], tr_tab[1]);
			break;
		case OBJ_HMAP:
			inst = (ax_one *)__ax_hmap_construct(tr_tab[0], tr_tab[1]);
			break;
		case OBJ_QUEUE:
			inst = (ax_one *)__ax_queue_construct(tr_tab[0]);
			break;
		case OBJ_STACK:
			inst = (ax_one *)__ax_stack_construct(tr_tab[0]);
			break;
		case OBJ_BTRIE:
			inst = (ax_one *)__ax_btrie_construct(tr_tab[0], tr_tab[1]);
			break;
	}

	return inst;
}
