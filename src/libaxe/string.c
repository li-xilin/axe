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
#include <axe/str.h>
#include <axe/iter.h>
#include <axe/scope.h>
#include <axe/base.h>
#include <axe/pool.h>
#include <axe/error.h>
#include <axe/log.h>
#include <alloca.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "check.h"

struct ax_string_st
{
	ax_str str;
	ax_vector *vector;
	ax_one_env one_env;
};

static void     iter_move(ax_iter *it, long i);
static void     iter_prev(ax_iter *it);
static void     iter_next(ax_iter *it);

static ax_fail  iter_less(const ax_iter *it1, const ax_iter *it2);
static long     iter_dist(const ax_iter *it1, const ax_iter *it2);
static ax_fail  iter_equal(const ax_iter *it1, const ax_iter *it2);

static void    *iter_get(const ax_iter *it);
static ax_fail  iter_set(const ax_iter *it, const void *val);
static void     iter_erase(ax_iter *it);

static void     riter_move(ax_iter *it, long i);
static void     riter_prev(ax_iter *it);
static void     riter_next(ax_iter *it);

static ax_fail  riter_less(const ax_iter *it1, const ax_iter *it2);
static long     riter_dist(const ax_iter *it1, const ax_iter *it2);



static ax_bool str_append(ax_str* str, const char *s);
static size_t  str_length (ax_str* str);
static ax_bool str_insert (ax_str* str, size_t start, const char *s);
static const char *str_cstr (ax_str* str);
static int     str_comp(ax_str* str, const char* s);
static ax_str *str_substr (ax_str* str, size_t start, size_t len);
static ax_seq *str_split (ax_str* str, const char ch);
static ax_fail str_sprintf(ax_str* str, const char *fmt, va_list args); 

static size_t  box_size(const ax_box* box);
static size_t  box_maxsize(const ax_box* box);
static ax_iter box_begin(const ax_box* box);
static ax_iter box_end(const ax_box* box);
static ax_iter box_rbegin(const ax_box* box);
static ax_iter box_rend(const ax_box* box);
static void    box_clear(ax_box* box);
static const ax_stuff_trait *box_elem_tr(const ax_box *box);

static void    any_dump(const ax_any* any, int ind);
static ax_any* any_copy(const ax_any* any);
static ax_any* any_move(ax_any* any);

static void        one_free(ax_one* one);
static ax_one_env *one_envp(const ax_one *one);

static const ax_one_trait one_trait;
static const ax_any_trait any_trait;
static const ax_box_trait box_trait;
static const ax_str_trait str_trait;
static const ax_iter_trait iter_trait;
static const ax_iter_trait riter_trait;

static void iter_move(ax_iter *it, long i)
{
	ax_box *box = (ax_box *)it->owner;
	ax_iter end = ax_box_end(box);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &end));
	end.point = it->point;
	end.tr->move(&end, i);
	it->point = end.point;
}

static void iter_prev(ax_iter *it)
{
	ax_box *box = (ax_box *)it->owner;
	ax_iter end = ax_box_end(box);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &end));
	end.point = it->point;
	end.tr->prev(it);
	it->point = end.point;
}

static void iter_next(ax_iter *it)
{
	ax_box *box = (ax_box *)it->owner;
	ax_iter end = ax_box_end(box);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &end));
	end.point = it->point;
	end.tr->prev(it);
	it->point = end.point;
}

static void *iter_get(const ax_iter *it)
{
	CHECK_PARAM_NULL(it);

	ax_box *box = (ax_box *)it->owner;
	ax_iter end = ax_box_end(box);
	end = ax_iter_prev(end);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &end));
	end.point = it->point;
	return end.tr->get(&end);
}

static ax_fail iter_set(const ax_iter *it, const void *val)
{
	CHECK_PARAM_NULL(it);

	ax_box *box = (ax_box *)it->owner;
	ax_iter end = ax_box_end(box);
	end = ax_iter_prev(end);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &end));
	end.point = it->point;
	return end.tr->set(it, val);

}

static void iter_erase(ax_iter *it)
{
	CHECK_PARAM_NULL(it);

	ax_box *box = (ax_box *)it->owner;
	ax_iter end = ax_box_end(box);
	end = ax_iter_prev(end);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &end));
	end.point = it->point;

	end.tr->erase(it);
}

static ax_bool iter_equal(const ax_iter *it1, const ax_iter *it2)
{
	return it1->point == it2->point;
}

static ax_bool iter_less(const ax_iter *it1, const ax_iter *it2)
{

	ax_box *box = (ax_box *)it1->owner;
	ax_iter end = ax_box_end(box);
	ax_iter it1_copy = *it1;
	ax_iter it2_copy = *it2;
	CHECK_ITERATOR_VALIDITY(it1, !iter_equal(&it1_copy, &end));
	CHECK_ITERATOR_VALIDITY(it2, !iter_equal(&it2_copy, &end));
	it1_copy.tr = it2_copy.tr = end.tr;
	return end.tr->less(&it1_copy, &it2_copy);
}

static long iter_dist(const ax_iter *it1, const ax_iter *it2)
{
	ax_box *box = (ax_box *)it1->owner;
	ax_iter end = ax_box_end(box);
	ax_iter it1_copy = *it1;
	ax_iter it2_copy = *it2;
	it1_copy.tr = it2_copy.tr = end.tr;
	return end.tr->dist(&it1_copy, &it2_copy);
}

static void riter_move(ax_iter *it, long i)
{
	ax_box *box = (ax_box *)it->owner;
	ax_iter end = ax_box_end(box);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &end));
	end.point = it->point;
	end.tr->move(&end, i);
	it->point = end.point;
}

static void riter_prev(ax_iter *it)
{
	ax_box *box = (ax_box *)it->owner;
	ax_iter rend = ax_box_rend(box);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &rend));
	rend.point = it->point;
	rend.tr->prev(it);
	it->point = rend.point;
}

static void riter_next(ax_iter *it)
{
	ax_box *box = (ax_box *)it->owner;
	ax_iter rend = ax_box_end(box);
	CHECK_ITERATOR_VALIDITY(it, !iter_equal(it, &rend));
	rend.point = it->point;
	rend.tr->prev(it);
	it->point = rend.point;
}

static ax_fail  riter_less(const ax_iter *it1, const ax_iter *it2)
{

	ax_box *box = (ax_box *)it1->owner;
	ax_iter rend = ax_box_rend(box);
	ax_iter it1_copy = *it1;
	ax_iter it2_copy = *it2;
	CHECK_ITERATOR_VALIDITY(it1, !ax_iter_equal(it1_copy, rend));
	CHECK_ITERATOR_VALIDITY(it2, !ax_iter_equal(it2_copy, rend));
	it1_copy.tr = it2_copy.tr = rend.tr;
	return rend.tr->less(&it1_copy, &it2_copy);
}

static long riter_dist(const ax_iter *it1, const ax_iter *it2)
{
	ax_box *box = (ax_box *)it1->owner;
	ax_iter rend = ax_box_rend(box);
	ax_iter it1_copy = *it1;
	ax_iter it2_copy = *it2;
	it1_copy.tr = it2_copy.tr = rend.tr;
	return rend.tr->dist(&it1_copy, &it2_copy);
}

static void one_free(ax_one* one)
{
	if (!one)
		return;

	ax_string_role role = { .one = one };
	ax_scope_detach(one);
	ax_one_free(ax_cast(vector, role.string->vector).one);
	ax_pool_free(one);
}

static ax_one_env *one_envp(const ax_one *one)
{
	CHECK_PARAM_NULL(one);
	ax_string_role role = { .one = (ax_one *)one };
	return &role.string->one_env;
}

static void any_dump(const ax_any* any, int ind)
{
	ax_pinfo("have not implemented");
}

static ax_any* any_copy(const ax_any* any)
{
	CHECK_PARAM_NULL(any);

	ax_string_role role = { .any = (ax_any*)any };
	ax_pool *pool = ax_base_pool(ax_one_base(role.one));
	ax_string* new = ax_pool_alloc(pool, (sizeof(ax_string)));
	memcpy(new, role.string, sizeof(ax_string));
	ax_vector_role srcvec_role = { .vector = role.string->vector };
	ax_vector_role dstvec_role = { .any = ax_any_copy(srcvec_role.any) };
	new->vector = dstvec_role.vector;

	return (ax_any*)new;
}

static ax_any* any_move(ax_any* any)
{
	CHECK_PARAM_NULL(any);

	ax_string_role role = { .any = (ax_any*)any };
	ax_pool *pool = ax_base_pool(ax_one_base(role.one));
	ax_string* new = ax_pool_alloc(pool, (sizeof(ax_string)));
	memcpy(new, role.string, sizeof(ax_string));
	ax_vector_role srcvec_role = { .vector = role.string->vector };
	ax_vector_role dstvec_role = { .any = ax_any_move(srcvec_role.any) };
	char zero = '\0';
	ax_seq_push(srcvec_role.seq, &zero);
	new->vector = dstvec_role.vector;

	return (ax_any*)new;
}

static size_t box_size(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_role role = { .box = (ax_box*)box };
	ax_vector_role vec_role = { .vector = role.string->vector };
	return ax_box_size(vec_role.box) - 1;
}

static size_t box_maxsize(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	return 0xFF;
}

static ax_iter box_begin(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_role role = { .box = (ax_box*)box};
	ax_vector_role vec_role = { .vector = role.string->vector };
	ax_iter it = ax_box_begin(vec_role.box);
	it.tr = &iter_trait;
	return it;
}

static ax_iter box_end(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_role role = { .box = (ax_box*)box};
	ax_vector_role vec_role = { .vector = role.string->vector };
	ax_iter it = ax_box_end(vec_role.box);
	it = ax_iter_prev(it);
	it.tr = &iter_trait;
	return it;
	
}

static ax_iter box_rbegin(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_role role = { .box = (ax_box*)box};
	ax_vector_role vec_role = { .vector = role.string->vector };
	ax_iter it = ax_box_rbegin(vec_role.box);
	it = ax_iter_next(it);
	it.tr = &riter_trait;
	return it;
}

static ax_iter box_rend(const ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_role role = { .box = (ax_box*)box};
	ax_vector_role vec_role = { .vector = role.string->vector };
	ax_iter it = ax_box_rend(vec_role.box);
	it.tr = &riter_trait;
	return it;
}


static void box_clear(ax_box* box)
{
	CHECK_PARAM_NULL(box);

	ax_string_role role = { .box = (ax_box*)box};
	ax_vector_role vec_role = { .vector = role.string->vector };
	ax_box_clear(vec_role.box);
}

static const ax_stuff_trait *box_elem_tr(const ax_box *box)
{
	ax_string_role role = { .box = (ax_box*)box };
	ax_vector_role vec_role = { .vector = (ax_vector *)role.string->vector };
	return ax_box_elem_tr(vec_role.box);
}

static ax_bool str_append(ax_str* str, const char *s)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_NULL(s);

	size_t len = strlen(s);
	ax_string_role role = { .str = str };
	ax_vector_role vec_role = { .vector = role.string->vector };
	size_t oldsize = ax_box_size(vec_role.box);
	ax_seq_trunc(vec_role.seq, (oldsize + len) * sizeof(char));
	char *buffer = ax_vector_buffer(vec_role.vector);
	memcpy(buffer + (oldsize - 1) * sizeof(char), s, (len + 1) * sizeof(char));
	
	return ax_true;
}

static size_t str_length (ax_str* str)
{
	CHECK_PARAM_NULL(str);
	
	ax_string_role role = { .str = str };
	ax_vector_role vec_role = { .vector = role.string->vector };
	return ax_box_size(vec_role.box) - 1;
}

static ax_bool str_insert (ax_str* str, size_t start, const char *s)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_VALIDITY(start, start < str_length(str));
	
	size_t len = strlen(s);
	ax_string_role role = { .str = str };
	ax_vector_role vec_role = { .vector = role.string->vector };
	size_t oldsize = ax_box_size(vec_role.box);
	ax_seq_trunc(vec_role.seq, oldsize + len + sizeof(char));
	char *buffer = ax_vector_buffer(vec_role.vector);
	memmove(buffer + start + len, buffer + start, oldsize - start);
	memcpy(buffer + start, s, len);

	return ax_true;
}

static const char *str_cstr (ax_str* str)
{
	CHECK_PARAM_NULL(str);

	ax_string_role role = { .str = str };
	ax_vector_role vec_role = { .vector = role.string->vector };
	return ax_vector_buffer(vec_role.vector);
}

static int str_comp(ax_str* str, const char* s)
{
	CHECK_PARAM_NULL(str);
	CHECK_PARAM_NULL(s);

	ax_string_role role = { .str = str };
	ax_vector_role vec_role = { .vector = role.string->vector };
	const char *buffer = ax_vector_buffer(vec_role.vector);
	return strcmp(buffer, s);
}

static ax_str *str_substr (ax_str* str, size_t start, size_t len)
{

	CHECK_PARAM_VALIDITY(start, start < ax_str_length(str));
	CHECK_PARAM_VALIDITY(len, start + len < ax_str_length(str));

	ax_string_role role = { .str = str };
	ax_vector_role vec_role = { .vector = role.string->vector };

	ax_string_role ret_role = ax_string_create(ax_one_scope(role.one));
	if (ret_role.one == NULL) {
		ax_base *base = ax_one_base(role.one);
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	char* buffer = ax_vector_buffer(vec_role.vector);
	char ch = buffer[start + len];
	buffer[start + len] = '\0';
	ax_str_append(ret_role.str, buffer + start);
	buffer[start + len] = ch;
	return ret_role.str;
}

static ax_seq *str_split (ax_str* str, const char ch)
{
	ax_string_role role = { .str = str };
	ax_vector_role vec_role = { .vector = role.string->vector };
	char *buffer = ax_vector_buffer(vec_role.vector);
	char *cur = buffer, *head = buffer;
	ax_base *base = ax_one_base(role.one);
	ax_vector_role ret_role = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_S));
	if (ret_role.one == NULL) {
		ax_base *base = ax_one_base(role.one);
		ax_base_set_errno(base, AX_ERR_ITACC);
		return NULL;
	}
	while (*cur) {
		if (*cur == ch) {
			char backup = *cur;
			*cur = '\0';
			if(ax_seq_push(ret_role.seq, head))
				goto failed;
			*cur = backup;
			head = cur + 1;
		}
		cur++;
	}
	if(ax_seq_push(ret_role.seq, head))
		goto failed;
	return ret_role.seq;
failed:
       if (ret_role.one)
	       ax_one_free(ret_role.one);
       return NULL;
}


static ax_fail str_sprintf(ax_str* str, const char *fmt, va_list args)
{
	const size_t max_size = 4096;
	char buf[max_size];
	int ret = vsnprintf(buf, max_size, fmt, args);
	if (ret == -1 || ret >= max_size) {
		ax_base *base = ax_one_base(&str->box.any.one);
		ax_base_set_errno(base, AX_ERR_TOOLONG);
	       return ax_true;	
	}
	if (ax_str_append(str, buf))
		return ax_true;
	return ax_false;
}

static const ax_one_trait one_trait =
{
	.name = "one.any.box.str:string",
	.free = one_free,
	.envp = one_envp
};

static const ax_any_trait any_trait =
{
	.dump = any_dump,
	.copy = any_copy,
	.move = any_move,
};

static const ax_box_trait box_trait =
{
	.size = box_size,
	.maxsize = box_maxsize,
	.elem_tr = box_elem_tr,

	.begin = box_begin,
	.end = box_end,
	.rbegin = box_rbegin,
	.rend = box_rend,

	.clear = box_clear,
};

static const ax_str_trait str_trait =
{
	.append = str_append,
	.length = str_length,
	.insert = str_insert,
	.cstr = str_cstr,
	.comp = str_comp,
	.substr = str_substr,
	.split = str_split,
	.sprintf = str_sprintf
};

static const ax_iter_trait iter_trait =
{
	.norm   = ax_true,
	.type   = AX_IT_RAND,

	.move   = iter_move,
	.prev = iter_prev,
	.next = iter_next,

	.get    = iter_get,
	.set    = iter_set,
	.erase  = iter_erase,

	.equal  = iter_equal,
	.less   = iter_less,
	.dist   = iter_dist
};

static const ax_iter_trait riter_trait =
{
	.norm   = ax_false,
	.type   = AX_IT_RAND,

	.move   = riter_move,
	.prev = riter_prev,
	.next = riter_next,

	.get    = iter_get,
	.set    = iter_set,
	.erase  = iter_erase,

	.equal  = iter_equal,
	.less   = riter_less,
	.dist   = riter_dist
};

ax_str *__ax_string_construct(ax_base* base)
{
	ax_string *string = ax_pool_alloc(ax_base_pool(base), sizeof(ax_string));
	if (string == NULL) {
		ax_base_set_errno(base, AX_ERR_NOMEM);
		return NULL;
	}

	ax_vector_role vec_role = { .seq = __ax_vector_construct(base, ax_stuff_traits(AX_ST_U8)) };
	ax_string string_init = {
		.str = {
			.box = {
				.any = {
					.one = {
						.tr = &one_trait
					},
					.tr = &any_trait,
				},
				.tr = &box_trait,
			},
			.tr = &str_trait
		},
		.one_env = {
			.base = base,
			.scope = NULL,
			.sindex = 0
		},
		.vector = vec_role.vector
	};
	char zero = '\0';
	ax_seq_push(vec_role.seq, &zero);

	memcpy(string, &string_init, sizeof string_init);
	return &string->str;
}


ax_string_role ax_string_create(ax_scope *scope)
{
	ax_string_role role = { .str = __ax_string_construct(ax_scope_base(scope)) };
	if (role.one == NULL)
		return role;
	ax_scope_attach(scope, role.one);
	return role;
}
