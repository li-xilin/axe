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

#include <ax/string.h>
#include <ax/vector.h>
#include <ax/buff.h>
#include <ax/str.h>
#include <ax/iter.h>
#include <ax/mem.h>
#include <ax/log.h>
#include <ax/dump.h>
#include <ax/trait.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "ax/class.h"
#include "check.h"

#define MIN_SIZE 16

ax_begin_data(string)
	ax_buff_r buff_r;
ax_end;

static void    citer_move(ax_citer *it, long i);
static void    citer_prev(ax_citer *it);
static void    citer_next(ax_citer *it);
static ax_fail citer_less(const ax_citer *it1, const ax_citer *it2);
static long    citer_dist(const ax_citer *it1, const ax_citer *it2);

static void    rciter_move(ax_citer *it, long i);
static void    rciter_prev(ax_citer *it);
static void    rciter_next(ax_citer *it);
static ax_fail rciter_less(const ax_citer *it1, const ax_citer *it2);
static long    rciter_dist(const ax_citer *it1, const ax_citer *it2);


static void   *citer_get(const ax_citer *it);
static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap);
static void    iter_erase(ax_iter *it);

static ax_fail str_append(ax_str *str, const char *s);
static ax_fail str_insert(ax_str* str, size_t start, const char *s);
static char   *str_strz(ax_str* str);
static int     str_comp(const ax_str* str, const char* s);
static ax_str *str_substr(const ax_str* str, size_t start, size_t len);
static ax_seq *str_split(const ax_str* str, const char ch);
static ax_fail str_sprintf(ax_str* str, const char *fmt, va_list args); 

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap);
static ax_fail seq_pop(ax_seq *seq);
static ax_fail seq_trunc(ax_seq *seq, size_t size);
static ax_iter seq_at(const ax_seq *seq, size_t index);
static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap);

static size_t  box_size(const ax_box* box);
static size_t  box_maxsize(const ax_box* box);
static ax_iter box_begin(ax_box* box);
static ax_iter box_end(ax_box* box);
static ax_iter box_rbegin(ax_box* box);
static ax_iter box_rend(ax_box* box);
static void    box_clear(ax_box* box);

static ax_dump*any_dump(const ax_any* any);
static ax_any* any_copy(const ax_any* any);

static void    one_free(ax_one* one);
static const char *one_name(const ax_one *one);

const ax_str_trait ax_string_tr;

#ifndef NDEBUG
bool iter_if_valid(const ax_citer *it);
bool iter_if_have_value(const ax_citer *it);

bool iter_if_valid(const ax_citer *it)
{
	const ax_string *self = it->owner;
	const ax_buff *buff = self->buff_r.buff;
	const ax_byte *ptr = ax_buff_cptr(buff);
	return ax_citer_norm(it)
		? (ax_byte *)it->point >= ptr && (ax_byte *)it->point <= ptr
				+ ax_buff_size(buff, NULL) - sizeof(char)
		: (ax_byte *)it->point >= ptr - sizeof(char)
				&& (ax_byte *)it->point <= ptr + ax_buff_size(buff, NULL) - 2 * sizeof(char);

}

bool iter_if_have_value(const ax_citer *it)
{
	const ax_string *self = it->owner;
	const ax_buff *buff = self->buff_r.buff;
	const ax_byte *ptr = ax_buff_cptr(buff);
	return (ax_byte *)it->point >= ptr
		&& (ax_byte *)it->point <= ptr + ax_buff_size(buff, NULL) - 2 * sizeof(char);
}

#endif

static void citer_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	it->point = (char *)it->point + i;
	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}

static void citer_prev(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	it->point = (char *)it->point - 1;
	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}

static void citer_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	it->point = (char *)it->point + 1;
	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}

static bool citer_less(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_PARAM_NULL(it1);
	CHECK_PARAM_NULL(it2);
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it2));

	return (char *)it1->point < (char *)it2->point;
}

static long citer_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_PARAM_NULL(it1);
	CHECK_PARAM_NULL(it2);
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it2));

	return (uintptr_t)((char *)it2->point - (char *)it1->point) / sizeof(char);
}

static void rciter_move(ax_citer *it, long i)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	it->point = (char *)it->point + i;
	CHECK_PARAM_VALIDITY(i, iter_if_valid(it));
}

static void rciter_prev(ax_citer *it)
{
	CHECK_PARAM_NULL(it);

	CHECK_PARAM_VALIDITY(it, iter_if_valid(it));
	it->point = (char *)it->point - 1;
	CHECK_PARAM_VALIDITY(it , iter_if_valid(it));
}

static void rciter_next(ax_citer *it)
{
	CHECK_PARAM_NULL(it);

	CHECK_PARAM_VALIDITY(it , iter_if_valid(it));
	it->point = (char *)it->point + 1;
	CHECK_PARAM_VALIDITY(it , iter_if_valid(it));
}

static ax_fail  rciter_less(const ax_citer *it1, const ax_citer *it2)
{

	CHECK_PARAM_NULL(it1);
	CHECK_PARAM_NULL(it2);
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it2));
	return (char *)it1->point > (char *)it2->point;
}

static long rciter_dist(const ax_citer *it1, const ax_citer *it2)
{
	CHECK_PARAM_NULL(it1);
	CHECK_PARAM_NULL(it2);
	CHECK_ITER_COMPARABLE(it1, it2);
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it1));
	CHECK_PARAM_VALIDITY(it1, iter_if_valid(it2));

	return (uintptr_t)((char *)it1->point - (char *)it2->point) / sizeof(char);
}

static void *citer_get(const ax_citer *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(it));
	
	return (char *)it->point;
}

static ax_fail iter_set(const ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_NULL(val);
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_cc(it)));

	*(char *)it->point = *(char *) val;
	return false;
}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));

	ax_string *self = it->owner;
	ax_buff *buff = self->buff_r.buff;

	size_t size = ax_buff_size(buff, NULL);
	size_t nchar = size / sizeof(char);
	char *ptr = ax_buff_ptr(buff);
	size_t off = (char *)it->point - ptr;

	memmove(ptr + off + 1, ptr + off, size - (off + 1) * sizeof(char));
	ptr[nchar - 2] = '\0';
	(void)ax_buff_adapt(buff, size - sizeof(char));

	it->point = ptr + off + (ax_iter_norm(it) ? 0 : 1);
}

static void one_free(ax_one* one)
{
	if (!one)
		return;

	ax_string *self = (ax_string *) one;
	ax_one_free(self->buff_r.one);
	free(one);
}

static const char *one_name(const ax_one *one)
{
	return ax_class_name(5, string);
}

static ax_dump *any_dump(const ax_any* any)
{
	ax_str_cr self = { .any = any };
	ax_dump *block = NULL, *strdmp = NULL;
	block = ax_dump_block(ax_one_name(self.one), 1);
	strdmp = ax_dump_str(ax_str_cstrz(self.str));
	ax_dump_bind(block, 0, strdmp);
	return block;
}

static ax_any* any_copy(const ax_any* any)
{
	CHECK_PARAM_NULL(any);

	ax_string_r self_r = { .any = (ax_any*)any };

	ax_buff_r new_buf_r = { NULL };
	ax_string_r new_str_r = { NULL };

	new_str_r.string = malloc((sizeof(ax_string)));
	if (!new_str_r.string) {
		goto fail;
	}

	new_buf_r.any = ax_any_copy(self_r.string->buff_r.any);
	if (!new_buf_r.any)
		goto fail;

	memcpy(new_str_r.string, self_r.string, sizeof(ax_string));

	new_str_r.string->str.env.seq.box.any.one.scope.macro = NULL;
	new_str_r.string->str.env.seq.box.any.one.scope.micro = 0;
	return new_str_r.any;
fail:
	ax_one_free(new_buf_r.one);
	ax_one_free(new_str_r.one);
	return NULL;
}

static size_t box_size(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_cr self_r = { .box = box };
	size_t bsize = ax_buff_size(self_r.string->buff_r.buff, NULL);
	return bsize / sizeof(char) - 1;
}

static size_t box_maxsize(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_cr self_r = { .box = box };
	size_t maxsize = ax_buff_max(self_r.string->buff_r.buff);
	return maxsize / sizeof(char) - 1;
}

static ax_iter box_begin(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = box };
	return (ax_iter) {
		.owner = box,
		.point = (ax_byte *)ax_buff_ptr(self_r.string->buff_r.buff),
		.tr = &ax_string_tr.seq.box.iter,
		.etr = box->env.elem_tr,
	};
}

static ax_iter box_end(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = box };
	ax_buff *buff = self_r.string->buff_r.buff;
	return (ax_iter) {
		.owner = box,
		.point = (ax_byte *)ax_buff_ptr(buff)
			+ ax_buff_size(buff, NULL) - sizeof(char),
		.tr = &ax_string_tr.seq.box.iter,
		.etr = box->env.elem_tr,
	};
}

static ax_iter box_rbegin(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = box };
	ax_buff *buff = self_r.string->buff_r.buff;
	return (ax_iter) {
		.owner = box,
		.point = (ax_byte *)ax_buff_ptr(buff)
			+ ax_buff_size(buff, NULL) - 2 * sizeof(char),
		.tr = &ax_string_tr.seq.box.riter,
		.etr = box->env.elem_tr,
	};
}

static ax_iter box_rend(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = box };
	return (ax_iter) {
		.owner = box,
		.point = (ax_byte *)ax_buff_ptr(self_r.string->buff_r.buff) - sizeof(char),
		.tr = &ax_string_tr.seq.box.riter,
		.etr = box->env.elem_tr,
	};
}

static void box_clear(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = (ax_box *)box };
	ax_buff *buff = self_r.string->buff_r.buff;

	(void)ax_buff_resize(buff, sizeof(char)); // Always success

	char *ptr = ax_buff_ptr(buff);
	ptr[0] = '\0';
}

static ax_fail str_append(ax_str* str, const char *s)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_NULL(s);

	ax_string_r self_r = { .str = str };
	ax_buff *buff = self_r.string->buff_r.buff;

	size_t append_len = strlen(s) * sizeof(char);
	size_t old_size = ax_buff_size(buff, NULL);
	size_t new_size = old_size + append_len;
	
	if (ax_buff_adapt(buff, new_size))
		goto fail;

	char *ptr = ax_buff_ptr(buff);
	memcpy(ptr + old_size - sizeof(char), s, append_len + sizeof(char));
	
	return false;
fail:
	return true;
}

static ax_fail str_insert(ax_str* str, size_t start, const char *s)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_VALIDITY(start, start < ax_box_size(ax_r(str, str).box));
	
	ax_string_r self_r = { .str = str };
	ax_buff *buff = self_r.string->buff_r.buff;

	size_t insert_len = strlen(s);
	size_t old_bsize = ax_buff_size(buff, NULL);
	ax_buff_adapt(buff, old_bsize + insert_len);
	char *ptr = ax_buff_ptr(buff);
	memmove(ptr + (start + insert_len), ptr + start, old_bsize - start);
	memcpy(ptr + start, s, insert_len);

	return true;
}

static char *str_strz(ax_str* str)
{
	CHECK_PARAM_NULL(str);

	ax_string_r self_r = { .str = str };
	ax_buff *buff = self_r.string->buff_r.buff;
	return ax_buff_ptr(buff);
}

static int str_comp(const ax_str* str, const char* s)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_NULL(s);

	ax_string_cr self_r = { .str = str };
	ax_buff *buff = self_r.string->buff_r.buff;
	char *ptr = (char*) ax_buff_ptr(buff);
	return strcmp(ptr, s);
}

static ax_str *str_substr (const ax_str* str, size_t start, size_t len)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_VALIDITY(start, start < ax_str_length(str));
	CHECK_PARAM_VALIDITY(len, start + len < ax_str_length(str));

	ax_string_cr self_r = { .str = str };
	ax_buff *buff = self_r.string->buff_r.buff;

	ax_string_r ret_r = { NULL };
	ret_r.str = __ax_string_construct();
	if (!ret_r.one) {
		goto fail;
	}

	char* buffer = ax_buff_ptr(buff);
	char back = buffer[start + len];
	buffer[start + len] = '\0';
	if (ax_str_append(ret_r.str, buffer + start))
		goto fail;
	buffer[start + len] = back;
	return ret_r.str;
fail:
	ax_one_free(ret_r.one);
	return NULL;
}

static ax_seq *str_split (const ax_str* str, const char ch)
{
	CHECK_PARAM_NULL(str);

	ax_string_cr self_r = { .str = str };
	ax_buff *buff = self_r.string->buff_r.buff;

	char *buffer = ax_buff_ptr(buff);
	char *cur = buffer, *head = buffer;

	ax_vector_r ret = ax_new(vector, ax_t(str));
	if (ret.one == NULL)
		goto fail;

	while (*cur) {
		if (*cur == ch) {
			char backup = *cur;
			*cur = '\0';
			if(ax_seq_push(ret.seq, head))
				goto fail;
			*cur = backup;
			head = cur + 1;
		}
		cur++;
	}
	if(ax_seq_push(ret.seq, head))
		goto fail;
	return ret.seq;
fail:
	ax_one_free(ret.one);
       return NULL;
}

static ax_fail str_sprintf(ax_str* str, const char *fmt, va_list args)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_NULL(fmt);

	const size_t max_size = 4096;
	char buf[max_size];
	int ret = vsnprintf(buf, max_size, fmt, args);
	if (ret == -1 || ret >= max_size) {
	       return true;	
	}
	if (ax_str_append(str, buf))
		return true;
	return false;
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(val);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner == seq && iter_if_valid(ax_iter_c(it)));

	ax_string *self = (ax_string *) seq;
	ax_buff *buff = self->buff_r.buff;

	char *ptr = ax_buff_ptr(buff);
	size_t old_size = ax_buff_size(self->buff_r.buff, NULL);

	long offset = (char *)it->point - ptr; //backup offset before realloc

	if (ax_buff_adapt(self->buff_r.buff, old_size + sizeof(char)))
		return true;

	ptr = ax_buff_ptr(self->buff_r.buff);
	it->point = ptr + offset; //restore offset

	size_t nchar = old_size / sizeof(char);
	size_t ins_off = ax_iter_norm(it) ? offset : offset + 1;
	memmove(ptr + ins_off + 1, ptr + ins_off, (nchar - offset) * sizeof(char));

	ptr[ins_off] = *(char *)val;

	if(ax_iter_norm(it))
		it->point = (ax_byte*)it->point + sizeof(char);
	return false;
}

static ax_fail seq_push(ax_seq *seq, const void *val, va_list *ap)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(val);

	if (!val || *(char *)val == '\0')
		return false;

	ax_string *self = (ax_string *) seq;

	size_t size = ax_buff_size(self->buff_r.buff, NULL);

	size += sizeof(char);
	if (ax_buff_adapt(self->buff_r.buff, size))
		return true;

	char *ptr = ax_buff_ptr(self->buff_r.buff);

	size_t nchar = size / sizeof(char);

	ptr[nchar - 2] = *(char *)val;
	ptr[nchar - 1] = '\0';

	return false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_string *self = (ax_string *) seq;
	size_t size = ax_buff_size(self->buff_r.buff, NULL);
	char *ptr = ax_buff_ptr(self->buff_r.buff);

	if (size == sizeof('\0')) {
		return false;
	}

	size -= sizeof(char);
	if (ax_buff_adapt(self->buff_r.buff, size))
		return true;
	size_t nchar = size / sizeof(char);
	ptr[nchar - 1] = '\0';

	return false;
}

static void seq_invert(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_string *self = (ax_string *) seq;
	size_t size = ax_buff_size(self->buff_r.buff, NULL);
	char *ptr = ax_buff_ptr(self->buff_r.buff);

	if (size == sizeof('\0'))
		return;

	size_t left = 0, right = size - sizeof('\0') - sizeof(char);
	while (right - left > sizeof(char)) {
		ax_swap(ptr + left, ptr + right, char);
		left += sizeof(char);
		right -= sizeof(char);
	}
}

static ax_fail seq_trunc(ax_seq *seq, size_t size)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(size, size <= ax_box_maxsize(ax_r(seq, seq).box));

	ax_string_r self_r = { .seq = seq };
	ax_buff *buff = self_r.string->buff_r.buff;

	size_t old_size = ax_str_length(self_r.str);

	if (size == old_size) 
		return false;


	if (ax_buff_adapt(buff, size + sizeof(char)))
		return true;
	char *ptr = ax_buff_ptr(buff);

	if (size < old_size)
		ptr[size - 1] =  '\0';
	else
		memset(ptr + old_size, '\0', size - old_size + sizeof(char));
	return false;
}

static ax_iter seq_at(const ax_seq *seq, size_t index)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_VALIDITY(index, index <= ax_box_size(ax_cr(seq, seq).box));

	ax_string_cr self_r = { .seq = seq };
	ax_buff *buff = self_r.string->buff_r.buff;

	char *ptr = ax_buff_ptr(buff);

	ax_iter it = {
		.owner = (void *)self_r.one,
		.point =  ptr + index,
		.tr = &ax_string_tr.seq.box.iter,
		.etr = seq->env.box.elem_tr,
	};

	CHECK_PARAM_VALIDITY(index, iter_if_have_value(ax_iter_c(&it)));
	return it;
}

const ax_str_trait ax_string_tr =
{
	.seq = {
		.box = {
			.any = {
				.one = {
					.name = one_name,
					.free = one_free,
				},
				.dump = any_dump,
				.copy = any_copy,
			},
			.iter = {
				.norm = true,
				.type = AX_IT_RAND,
				.move = citer_move,
				.prev = citer_prev,
				.next = citer_next,
				.less = citer_less,
				.dist = citer_dist,
				.get    = citer_get,
				.set    = iter_set,
				.erase  = iter_erase,
			},

			.riter = {
				.norm = false,
				.type = AX_IT_RAND,
				.move = rciter_move,
				.prev = rciter_prev,
				.next = rciter_next,
				.less = rciter_less,
				.dist = rciter_dist,
				.get   = citer_get,
				.set   = iter_set,
				.erase = iter_erase,
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
		.invert = seq_invert,
		.trunc = seq_trunc,
		.at = seq_at,
		.insert = seq_insert,
	},
	.append = str_append,
	.insert = str_insert,
	.strz   = str_strz,
	.comp   = str_comp,
	.substr = str_substr,
	.split  = str_split,
	.sprintf = str_sprintf,
};

ax_str *__ax_string_construct()
{
	ax_string *self = NULL;
	ax_buff_r buff_r = { NULL };

	self = malloc(sizeof(ax_string));
	if (self == NULL)
		goto fail;

	buff_r.any = __ax_buff_construct();
	if (!buff_r.any)
		goto fail;

	ax_string string_init = {
		.str = {
			.tr = &ax_string_tr,
			.env.seq.box.elem_tr = ax_t(char)
		},
		.buff_r = buff_r,
	};
	char zero = '\0';
	ax_buff_reserve(buff_r.buff, MIN_SIZE * sizeof(char));
	ax_buff_resize(buff_r.buff, sizeof(char));
	char *ptr = ax_buff_ptr(buff_r.buff);
	ptr[0] = zero;

	memcpy(self, &string_init, sizeof string_init);
	return ax_r(string, self).str;
fail:
	ax_one_free(buff_r.one);
	return NULL;
}

