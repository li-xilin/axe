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

#include "ax/btrie.h"
#include "ax/avl.h"
#include "ax/list.h"
#include "ax/oper.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

struct check_table_st
{
	int index;
	struct {
		char *key;
		int value;
	} table[32];
};

static ax_btrie_r make_test_btrie(ax_base *base);
static void seq_to_path(const ax_seq *seq, char * path);

static void seq_to_path(const ax_seq *seq, char * path)
{
	strcpy(path, "/");
	ax_box_cforeach(ax_cr(seq, seq).box, const int *, val) {
			char num[32];
			sprintf(num, "%d/", *val);
			strcat(path, num);
	}
}

static void create(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_btrie_r btrie = ax_btrie_create(ax_base_local(base),
			ax_stuff_traits(AX_ST_I32), ax_stuff_traits(AX_ST_I32));
	axut_assert_uint_equal(r, ax_box_size(btrie.box), 0);
}

static void trie_put(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	
	int d = ax_base_enter(base);

	ax_btrie_r btrie = make_test_btrie(base);

	axut_assert_uint_equal(r, ax_box_size(btrie.box), 7);

	ax_base_leave(base, d);
}

struct enum_context_st {
	axut_runner *runner;
	struct check_table_st *check_table;
};

bool iterator_enum_cb(ax_trie *trie, const ax_seq *key, const void *val, void *ctx)
{
	struct enum_context_st *ectx = ctx;
	char path[1024];
	seq_to_path(key, path);
	char *ex_key = ectx->check_table->table[ectx->check_table->index].key;
	int ex_value = ectx->check_table->table[ectx->check_table->index].value;
	axut_assert_str_equal(ectx->runner, ex_key, path);
	axut_assert_int_equal(ectx->runner, ex_value, *(int *)val);
	ectx->check_table->index ++;
	return false;
}

static ax_btrie_r make_test_btrie(ax_base *base) {
	ax_btrie_r btrie = ax_btrie_create(ax_base_local(base),
			ax_stuff_traits(AX_ST_I32),
			ax_stuff_traits(AX_ST_I32));

	ax_list_r key = ax_list_create(ax_base_local(base),
			ax_stuff_traits(AX_ST_I32));

	int value;

	ax_seq_pushl(key.seq, "i32x3", 1, 1, 1);
	value = 111;
	ax_trie_put(btrie.trie, key.seq, &value);

	ax_box_clear(key.box);
	ax_seq_pushl(key.seq, "i32x3", 1, 2, 1);
	value = 121;
	ax_trie_put(btrie.trie, key.seq, &value);

	ax_box_clear(key.box);
	ax_seq_pushl(key.seq, "i32x1", 1);
	value = 1;
	ax_trie_put(btrie.trie, key.seq, &value);

	ax_box_clear(key.box);
	ax_seq_pushl(key.seq, "i32x3", 1, 1, 2);
	value = 112;
	ax_trie_put(btrie.trie, key.seq, &value);

	value = 0;
	ax_box_clear(key.box);
	ax_trie_put(btrie.trie, key.seq, &value);

	ax_box_clear(key.box);
	ax_seq_pushl(key.seq, "i32x3", 2, 1, 1);
	value = 211;
	ax_trie_put(btrie.trie, key.seq, &value);

	ax_box_clear(key.box);
	ax_seq_pushl(key.seq, "i32x2", 1, 1);
	value = 11;
	ax_trie_put(btrie.trie, key.seq, &value);

	return btrie;
}

static void iterater(axut_runner *r)
{

	ax_base *base = axut_runner_arg(r);
	
	ax_btrie_r btrie = make_test_btrie(base);

	struct check_table_st data = {
		.index = 0,
		.table = {
			{ "/1/1/1/", 111 },
			{ "/1/1/2/", 112 },
			{ "/1/1/", 11 },
			{ "/1/2/1/", 121 },
			{ "/1/", 1 },
			{ "/2/1/1/", 211 },
			{ "/", 0 },
			{ "", 0 },
		}
	};

	struct enum_context_st enumctx = {
		.check_table = &data,
		.runner = r,
	};
	ax_trie_enum(btrie.trie, iterator_enum_cb, &enumctx);
}

static void trie_get(axut_runner *r)
{

	ax_base *base = axut_runner_arg(r);

	int d = ax_base_enter(base);

	ax_btrie_r btrie = make_test_btrie(base);
	ax_list_r key;

	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert_int_equal(r, 111, *(int32_t*)ax_trie_get(btrie.trie, key.seq));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 2))
		axut_term(r, "ax_seq_pushl");
	axut_assert_int_equal(r, 112, *(int32_t*)ax_trie_get(btrie.trie, key.seq));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x2", 1, 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert_int_equal(r, 11, *(int32_t*)ax_trie_get(btrie.trie, key.seq));

	ax_box_clear(key.box);
	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));
	if (ax_seq_pushl(key.seq, "i32x3", 1, 2, 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert_int_equal(r, 121, *(int32_t*)ax_trie_get(btrie.trie, key.seq));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x1", 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert_int_equal(r, 1, *(int32_t*)ax_trie_get(btrie.trie, key.seq));

	ax_box_clear(key.box);
	axut_assert_int_equal(r, 0, *(int32_t*)ax_trie_get(btrie.trie, key.seq));

	ax_base_leave(base, d);
}

static void trie_at(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);

	int d = ax_base_enter(base);

	ax_btrie_r btrie = make_test_btrie(base);
	ax_list_r key;
	ax_iter it;
	ax_iter end;

	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 1))
		axut_term(r, "ax_seq_pushl");
	it = ax_trie_at(btrie.trie, key.seq);
	axut_assert_int_equal(r, 111, *(int32_t*)ax_iter_get(&it));
	axut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 2))
		axut_term(r, "ax_seq_pushl");
	it = ax_trie_at(btrie.trie, key.seq);
	axut_assert_int_equal(r, 112, *(int32_t*)ax_iter_get(&it));
	axut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x2", 1, 1))
		axut_term(r, "ax_seq_pushl");
	it = ax_trie_at(btrie.trie, key.seq);
	axut_assert_int_equal(r, 11, *(int32_t*)ax_iter_get(&it));
	axut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x1", 1))
		axut_term(r, "ax_seq_pushl");
	it = ax_trie_at(btrie.trie, key.seq);
	axut_assert_int_equal(r, 1, *(int32_t*)ax_iter_get(&it));
	axut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x2", 1, 2))
		axut_term(r, "ax_seq_pushl");
	it = ax_trie_at(btrie.trie, key.seq);
	end = ax_box_end(btrie.box);
	axut_assert(r, !ax_iter_equal(&it, &end));
	axut_assert(r, !ax_trie_iter_valued(&it));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x4", 1, 1, 1, 1))
		axut_term(r, "ax_seq_pushl");
	it = ax_trie_at(btrie.trie, key.seq);
	end = ax_box_end(btrie.box);
	axut_assert(r, ax_iter_equal(&it, &end));

	ax_base_leave(base, d);
}

static void trie_exist(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);

	int d = ax_base_enter(base);

	ax_btrie_r btrie = make_test_btrie(base);
	ax_list_r key;

	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));

	bool valued;

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 2))
		axut_term(r, "ax_seq_pushl");
	axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x2", 1, 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));
	if (ax_seq_pushl(key.seq, "i32x3", 1, 2, 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x1", 1))
		axut_term(r, "ax_seq_pushl");
	axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));


	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x2", 1, 2))
		axut_term(r, "ax_seq_pushl");
	axut_assert(r, ax_trie_exist(btrie.trie, key.seq, &valued));
	axut_assert(r, !valued);

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 2, 2))
		axut_term(r, "ax_seq_pushl");
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_base_leave(base, d);

}

static void trie_erase(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);

	int d = ax_base_enter(base);

	ax_btrie_r btrie = make_test_btrie(base);
	ax_list_r key;

	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 1))
		axut_term(r, "ax_seq_pushl");
	ax_trie_erase(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 2))
		axut_term(r, "ax_seq_pushl");
	ax_trie_erase(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x2", 1, 1))
		axut_term(r, "ax_seq_pushl");
	ax_trie_erase(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));
	if (ax_seq_pushl(key.seq, "i32x3", 1, 2, 1))
		axut_term(r, "ax_seq_pushl");
	ax_trie_erase(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x1", 1))
		axut_term(r, "ax_seq_pushl");
	ax_trie_erase(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_box_clear(key.box);
	ax_trie_erase(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

	ax_base_leave(base, d);

}

static void trie_prune(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);

	int d = ax_base_enter(base);

	ax_btrie_r btrie = make_test_btrie(base);
	ax_list_r key;

	key = ax_list_create(ax_base_local(base), ax_trie_key_tr(btrie.trie));

	btrie = make_test_btrie(base);
	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 1))
		axut_term(r, "ax_seq_pushl");
	ax_trie_prune(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));
	axut_assert_uint_equal(r, 6, ax_box_size(btrie.box));

	btrie = make_test_btrie(base);
	ax_box_clear(key.box);
	if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 2))
		axut_term(r, "ax_seq_pushl");
	ax_trie_prune(btrie.trie, key.seq);
	axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));
	axut_assert_uint_equal(r, 6, ax_box_size(btrie.box));

	{
		btrie = make_test_btrie(base);
		ax_box_clear(key.box);
		if (ax_seq_pushl(key.seq, "i32x2", 1, 1))
			axut_term(r, "ax_seq_pushl");
		ax_trie_prune(btrie.trie, key.seq);
		axut_assert_uint_equal(r, 4, ax_box_size(btrie.box));

		ax_box_clear(key.box);
		if (ax_seq_pushl(key.seq, "i32x2", 1, 1))
			axut_term(r, "ax_seq_pushl");
		axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

		ax_box_clear(key.box);
		if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 1))
			axut_term(r, "ax_seq_pushl");
		axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));

		ax_box_clear(key.box);
		if (ax_seq_pushl(key.seq, "i32x3", 1, 1, 2))
			axut_term(r, "ax_seq_pushl");
		axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));
	}

	{
		btrie = make_test_btrie(base);
		ax_box_clear(key.box);
		if (ax_seq_pushl(key.seq, "i32x1", 1))
			axut_term(r, "ax_seq_pushl");
		ax_trie_prune(btrie.trie, key.seq);
		axut_assert_uint_equal(r, 2, ax_box_size(btrie.box));

		ax_box_clear(key.box);
		axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));

		ax_box_clear(key.box);
		if (ax_seq_pushl(key.seq, "i32x3", 2, 1, 1))
			axut_term(r, "ax_seq_pushl");
		axut_assert(r, ax_trie_exist(btrie.trie, key.seq, NULL));
	}

	{
		btrie = make_test_btrie(base);
		ax_box_clear(key.box);
		ax_trie_prune(btrie.trie, key.seq);
		axut_assert_uint_equal(r, 0, ax_box_size(btrie.box));

		ax_box_clear(key.box);
		axut_assert(r, !ax_trie_exist(btrie.trie, key.seq, NULL));
	}

	ax_base_leave(base, d);
}

static void trie_rekey(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	axut_assert(r, !"BUG");
}

static void clean(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_base_destroy(base);
}

axut_suite *suite_for_btrie(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "btrie");

	ax_base *base1 = ax_base_create();
	axut_suite_set_arg(suite, base1);

	axut_suite_add(suite, create, 0);
	axut_suite_add(suite, trie_put, 1);
	axut_suite_add(suite, trie_get, 1);
	axut_suite_add(suite, trie_exist, 2);
	axut_suite_add(suite, trie_at, 2);
	axut_suite_add(suite, trie_prune, 2);
	axut_suite_add(suite, trie_rekey, 2);
	axut_suite_add(suite, iterater, 2);

	axut_suite_add(suite, clean, 255);
	return suite;
}
