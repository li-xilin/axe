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

#include <axe/string.h>
#include <axe/vector.h>
#include <axe/buff.h>
#include <axe/str.h>
#include <axe/iter.h>
#include <axe/scope.h>
#include <axe/base.h>
#include <axe/mem.h>
#include <axe/pool.h>
#include <axe/error.h>
#include <axe/log.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "check.h"

#define MIN_SIZE 16

struct ax_string_st
{
	ax_str _str;
	ax_buff_r buff_r;
};

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


static void   *iter_get(const ax_iter *it);
static ax_fail iter_set(const ax_iter *it, const void *val);
static void    iter_erase(ax_iter *it);

static ax_fail str_append(ax_str *str, const char *s);
static size_t  str_length(const ax_str *str);
static ax_fail str_insert(ax_str* str, size_t start, const char *s);
static char   *str_strz(ax_str* str);
static int     str_comp(const ax_str* str, const char* s);
static ax_str *str_substr(const ax_str* str, size_t start, size_t len);
static ax_seq *str_split(const ax_str* str, const char ch);
static ax_fail str_sprintf(ax_str* str, const char *fmt, va_list args); 

static ax_fail seq_push(ax_seq *seq, const void *val);
static ax_fail seq_pop(ax_seq *seq);
static ax_fail seq_trunc(ax_seq *seq, size_t size);
static ax_iter seq_at(const ax_seq *seq, size_t index);
static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val);

static size_t  box_size(const ax_box* box);
static size_t  box_maxsize(const ax_box* box);
static ax_iter box_begin(ax_box* box);
static ax_iter box_end(ax_box* box);
static ax_iter box_rbegin(ax_box* box);
static ax_iter box_rend(ax_box* box);
static void    box_clear(ax_box* box);
static const ax_stuff_trait *box_elem_tr(const ax_box *box);

static void    any_dump(const ax_any* any, int ind);
static ax_any* any_copy(const ax_any* any);
static ax_any* any_move(ax_any* any);

static void    one_free(ax_one* one);

static const ax_str_trait str_trait;
static const ax_iter_trait iter_trait;
static const ax_iter_trait riter_trait;

#ifdef AX_DEBUG
ax_bool iter_if_valid(const ax_citer *it);
ax_bool iter_if_have_value(const ax_citer *it);

ax_bool iter_if_valid(const ax_citer *it)
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

ax_bool iter_if_have_value(const ax_citer *it)
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

static ax_bool citer_less(const ax_citer *it1, const ax_citer *it2)
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

	return ((char *)it2->point - (char *)it1->point) / sizeof(char);
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

	return ((char *)it1->point - (char *)it2->point) / sizeof(char);
}

static void *iter_get(const ax_iter *it)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));
	
	return (char *)it->point;

}

static ax_fail iter_set(const ax_iter *it, const void *val)
{
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, iter_if_have_value(ax_iter_c(it)));

	*(char *)it->point = *(char *) val;
	return ax_false;
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
	ax_scope_detach(one);
	ax_one_free(self->buff_r.one);
	ax_pool_free(one);
}

static void any_dump(const ax_any* any, int ind)
{
	ax_pinfo("have not implemented");
}

static ax_any* any_copy(const ax_any* any)
{
	CHECK_PARAM_NULL(any);

	ax_string_r self_r = { .any = (ax_any*)any };
	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);

	ax_buff_r new_buf_r = { NULL };
	ax_string_r new_str_r = { NULL };

	new_str_r.string = ax_pool_alloc(pool, (sizeof(ax_string)));
	if (!new_str_r.string) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	new_buf_r.any = ax_any_copy(self_r.string->buff_r.any);
	if (!new_buf_r.any)
		goto fail;

	memcpy(new_str_r.string, self_r.string, sizeof(ax_string));

	new_str_r.string->_str.env.one.scope.macro = NULL;
	new_str_r.string->_str.env.one.scope.micro = 0;
	ax_scope_attach(ax_base_local(base), new_str_r.one);

	return new_str_r.any;
fail:
	ax_one_free(new_buf_r.one);
	ax_one_free(new_str_r.one);
	return NULL;
}

static ax_any* any_move(ax_any* any)
{
	CHECK_PARAM_NULL(any);

	ax_string_r self_r = { .any = (ax_any*)any };
	ax_base *base = ax_one_base(self_r.one);
	ax_pool *pool = ax_base_pool(base);

	ax_buff_r new_buf_r = { NULL };
	ax_string_r new_str_r = { NULL };

	new_str_r.string = ax_pool_alloc(pool, (sizeof(ax_string)));
	if (!new_str_r.string) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	new_buf_r.any = ax_any_move(self_r.string->buff_r.any);
	if (!new_buf_r.any)
		goto fail;

	memcpy(new_str_r.string, self_r.string, sizeof(ax_string));

	new_str_r.string->_str.env.one.scope.macro = NULL;
	new_str_r.string->_str.env.one.scope.micro = 0;
	ax_scope_attach(ax_base_local(base), new_str_r.one);

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
		.tr = &iter_trait,
		.point = (ax_byte *)ax_buff_ptr(self_r.string->buff_r.buff)
	};
}

static ax_iter box_end(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = box };
	ax_buff *buff = self_r.string->buff_r.buff;
	return (ax_iter) {
		.owner = box,
		.tr = &iter_trait,
		.point = (ax_byte *)ax_buff_ptr(buff) + ax_buff_size(buff, NULL) - sizeof(char)
	};
}

static ax_iter box_rbegin(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = box };
	ax_buff *buff = self_r.string->buff_r.buff;
	return (ax_iter) {
		.owner = box,
		.tr = &riter_trait,
		.point = (ax_byte *)ax_buff_ptr(buff) + ax_buff_size(buff, NULL) - 2 * sizeof(char)
	};
}

static ax_iter box_rend(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_r self_r = { .box = box };
	return (ax_iter) {
		.owner = box,
		.tr = &riter_trait,
		.point = (ax_byte *)ax_buff_ptr(self_r.string->buff_r.buff) - sizeof(char)
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

static const ax_stuff_trait *box_elem_tr(const ax_box *box)
{
	ax_string_r self_r = { .box = (ax_box *)box };
	return self_r.seq->env.elem_tr;
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
	
	return ax_false;
fail:
	return ax_true;
}

static size_t str_length(const ax_str* str)
{
	CHECK_PARAM_NULL(str);
	ax_string_cr self_r = { .str = str };
	return ax_box_size(self_r.box);
}

static ax_fail str_insert(ax_str* str, size_t start, const char *s)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_VALIDITY(start, start < str_length(str));
	
	ax_string_r self_r = { .str = str };
	ax_buff *buff = self_r.string->buff_r.buff;

	size_t insert_len = strlen(s);
	size_t old_bsize = ax_buff_size(buff, NULL);
	ax_buff_adapt(buff, old_bsize + insert_len);
	char *ptr = ax_buff_ptr(buff);
	memmove(ptr + (start + insert_len), ptr + start, old_bsize - start);
	memcpy(ptr + start, s, insert_len);

	return ax_true;
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
	ax_base *base = ax_one_base(self_r.one);

	ax_string_r ret_r = { NULL };
	ret_r = ax_string_create(ax_base_local(base));
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
	ax_base *base = ax_one_base(self_r.one);

	char *buffer = ax_buff_ptr(buff);
	char *cur = buffer, *head = buffer;

	ax_vector_r ret_r = { NULL };
	ret_r = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_S));
	if (ret_r.one == NULL)
		goto fail;

	while (*cur) {
		if (*cur == ch) {
			char backup = *cur;
			*cur = '\0';
			if(ax_seq_push(ret_r.seq, head))
				goto fail;
			*cur = backup;
			head = cur + 1;
		}
		cur++;
	}
	if(ax_seq_push(ret_r.seq, head))
		goto fail;
	return ret_r.seq;
fail:
	ax_one_free(ret_r.one);
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
		ax_base *base = ax_one_base(ax_r(str, str).one);
		ax_base_set_errno(base, AX_ERR_TOOLONG);
	       return ax_true;	
	}
	if (ax_str_append(str, buf))
		return ax_true;
	return ax_false;
}

static ax_fail seq_insert(ax_seq *seq, ax_iter *it, const void *val)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(it);
	CHECK_PARAM_VALIDITY(it, it->owner == seq && iter_if_valid(ax_iter_c(it)));

	if (!val || *(char *)val == '\0')
		return ax_false;

	ax_string *self = (ax_string *) seq;
	ax_buff *buff = self->buff_r.buff;

	char *ptr = ax_buff_ptr(buff);
	size_t old_size = ax_buff_size(self->buff_r.buff, NULL);

	long offset = (char *)it->point - ptr; //backup offset before realloc

	if (ax_buff_adapt(self->buff_r.buff, old_size + sizeof(char)))
		return ax_true;

	ptr = ax_buff_ptr(self->buff_r.buff);
	it->point = ptr + offset; //restore offset

	size_t nchar = old_size / sizeof(char);
	size_t ins_off = ax_iter_norm(it) ? offset : offset + 1;
	memmove(ptr + ins_off + 1, ptr + ins_off, (nchar - offset) * sizeof(char));

	ptr[ins_off] = *(char *)val;

	if(ax_iter_norm(it))
		it->point = (ax_byte*)it->point + sizeof(char);
	return ax_false;
}

static ax_fail seq_push(ax_seq *seq, const void *val)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(val);

	if (!val || *(char *)val == '\0')
		return ax_false;

	ax_string *self = (ax_string *) seq;

	size_t size = ax_buff_size(self->buff_r.buff, NULL);

	size += sizeof(char);
	if (ax_buff_adapt(self->buff_r.buff, size))
		return ax_true;

	char *ptr = ax_buff_ptr(self->buff_r.buff);

	size_t nchar = size / sizeof(char);

	ptr[nchar - 2] = *(char *)val;
	ptr[nchar - 1] = '\0';

	return ax_false;
}

static ax_fail seq_pop(ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_string *self = (ax_string *) seq;
	size_t size = ax_buff_size(self->buff_r.buff, NULL);
	char *ptr = ax_buff_ptr(self->buff_r.buff);

	if (size == sizeof('\0')) {
		ax_base *base = ax_one_base(ax_r(string, self).one);
		ax_base_set_errno(base, AX_ERR_EMPTY);
		return ax_false;
	}

	size -= sizeof(char);
	if (ax_buff_adapt(self->buff_r.buff, size))
		return ax_true;
	size_t nchar = size / sizeof(char);
	ptr[nchar - 1] = '\0';

	return ax_false;
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
		ax_mem_pswap(ptr + left, ptr + right, char);
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
		return ax_false;


	if (ax_buff_adapt(buff, size + sizeof(char)))
		return ax_true;
	char *ptr = ax_buff_ptr(buff);

	if (size < old_size)
		ptr[size - 1] =  '\0';
	else
		memset(ptr + old_size, '\0', size - old_size + sizeof(char));
	return ax_false;
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
		.tr = &iter_trait,
		.point =  ptr + index
	};

	CHECK_PARAM_VALIDITY(index, iter_if_have_value(ax_iter_c(&it)));
	return it;
}

static const ax_str_trait str_trait =
{
	.seq = {
		.box = {
			.any = {
				.one = {
					.name = AX_STRING_NAME,
					.free = one_free,
				},
				.dump = any_dump,
				.copy = any_copy,
				.move = any_move,
			},
			.size = box_size,
			.maxsize = box_maxsize,
			.elem_tr = box_elem_tr,

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
	.length = str_length,
	.insert = str_insert,
	.strz   = str_strz,
	.comp   = str_comp,
	.substr = str_substr,
	.split  = str_split,
	.sprintf = str_sprintf
};

static const ax_iter_trait iter_trait =
{
	.ctr = {
		.norm   = ax_true,
		.type   = AX_IT_RAND,
		.move   = citer_move,
		.prev   = citer_prev,
		.next   = citer_next,
		.less   = citer_less,
		.dist   = citer_dist
	},

	.get    = iter_get,
	.set    = iter_set,
	.erase  = iter_erase,
};

static const ax_iter_trait riter_trait =
{
	.ctr = {
		.norm   = ax_false,
		.type   = AX_IT_RAND,
		.move   = rciter_move,
		.prev   = rciter_prev,
		.next   = rciter_next,
		.less   = rciter_less,
		.dist   = rciter_dist
	},
	.get    = iter_get,
	.set    = iter_set,
	.erase  = iter_erase
};

ax_str *__ax_string_construct(ax_base* base)
{
	CHECK_PARAM_NULL(base);
	
	ax_string *self = NULL;
	ax_buff_r buff_r = { NULL };

	self = ax_pool_alloc(ax_base_pool(base), sizeof(ax_string));
	if (self == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		goto fail;
	}

	buff_r.any = __ax_buff_construct(base);
	if (!buff_r.any)
		goto fail;

	ax_string string_init = {
		._str = {
			.tr = &str_trait,
			.env = {
				.one = {
					.base = base,
					.scope = { NULL },
				},
				.elem_tr = ax_stuff_traits(AX_ST_I8)
			},
		},
		.buff_r = buff_r
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
	ax_pool_free(self);
	return NULL;
}

ax_string_r ax_string_create(ax_scope *scope)
{
	ax_base *base = ax_one_base(ax_r(scope, scope).one);
	ax_string_r string_r = { .str = __ax_string_construct(base) };
	if (string_r.one == NULL)
		return string_r;
	ax_scope_attach(scope, string_r.one);
	return string_r;
}
