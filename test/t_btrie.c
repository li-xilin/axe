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
#include "ut/runner.h"
#include "ut/suite.h"

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

static ax_btrie_r make_test_btrie();
static void seq_to_path(const ax_seq *seq, char * path);

static void seq_to_path(const ax_seq *seq, char * path)
{
	strcpy(path, "/");
	ax_box_cforeach(ax_cr(ax_seq, seq).ax_box, const int *, val) {
			char num[32];
			sprintf(num, "%d/", *val);
			strcat(path, num);
	}
}

static void create(ut_runner *r)
{
	ax_btrie_r btrie = ax_new(ax_btrie, ax_t(i32), ax_t(i32));
	ut_assert_uint_equal(r, ax_box_size(btrie.ax_box), 0);
	ax_one_free(btrie.ax_one);
}

static void trie_put(ut_runner *r)
{
	
	ax_btrie_r btrie = make_test_btrie();
	ut_assert_uint_equal(r, ax_box_size(btrie.ax_box), 7);
	ax_one_free(btrie.ax_one);
}

struct enum_context_st {
	ut_runner *runner;
	struct check_table_st *check_table;
};

bool iterator_enum_cb(const ax_trie *trie, const ax_seq *key, const void *val, void *ctx)
{
	struct enum_context_st *ectx = ctx;
	char path[1024];
	seq_to_path(key, path);
	char *ex_key = ectx->check_table->table[ectx->check_table->index].key;
	int ex_value = ectx->check_table->table[ectx->check_table->index].value;
	ut_assert_str_equal(ectx->runner, ex_key, path);
	ut_assert_int_equal(ectx->runner, ex_value, *(int *)val);
	ectx->check_table->index ++;
	return false;
}

static ax_btrie_r make_test_btrie() {
	ax_btrie_r btrie = ax_new(ax_btrie, ax_t(int), ax_t(int));
	ax_list_r key = ax_new(ax_list, ax_t(int));

	int value;

	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1));
	value = 111;
	ax_trie_put(btrie.ax_trie, key.ax_seq, &value);

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 2, 1));
	value = 121;
	ax_trie_put(btrie.ax_trie, key.ax_seq, &value);

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1));
	value = 1;
	ax_trie_put(btrie.ax_trie, key.ax_seq, &value);

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 2));
	value = 112;
	ax_trie_put(btrie.ax_trie, key.ax_seq, &value);

	value = 0;
	ax_box_clear(key.ax_box);
	ax_trie_put(btrie.ax_trie, key.ax_seq, &value);

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 2, 1, 1));
	value = 211;
	ax_trie_put(btrie.ax_trie, key.ax_seq, &value);

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1));
	value = 11;
	ax_trie_put(btrie.ax_trie, key.ax_seq, &value);

	ax_one_free(key.ax_one);
	return btrie;
}

static void iterater(ut_runner *r)
{
	ax_btrie_r btrie = make_test_btrie();

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
	ax_trie_enum(btrie.ax_trie, iterator_enum_cb, &enumctx);
	ax_one_free(btrie.ax_one);
}

static void trie_get(ut_runner *r)
{

	ax_btrie_r btrie = make_test_btrie();
	ax_list_r key = ax_new(ax_list, ax_trie_key_tr(btrie.ax_trie));

	ax_box_clear(key.ax_box);

	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1));
	ut_assert_int_equal(r, 111, *(int32_t*)ax_trie_get(btrie.ax_trie, key.ax_seq));

	ax_box_clear(key.ax_box);

	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 2));
	ut_assert_int_equal(r, 112, *(int32_t*)ax_trie_get(btrie.ax_trie, key.ax_seq));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1));
	ut_assert_int_equal(r, 11, *(int32_t*)ax_trie_get(btrie.ax_trie, key.ax_seq));

	ax_box_clear(key.ax_box);

	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 2, 1));
	ut_assert_int_equal(r, 121, *(int32_t*)ax_trie_get(btrie.ax_trie, key.ax_seq));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1));
	ut_assert_int_equal(r, 1, *(int32_t*)ax_trie_get(btrie.ax_trie, key.ax_seq));

	ax_box_clear(key.ax_box);
	ut_assert_int_equal(r, 0, *(int32_t*)ax_trie_get(btrie.ax_trie, key.ax_seq));

	ax_one_free(btrie.ax_one);
	ax_one_free(key.ax_one);
}

static void trie_at(ut_runner *r)
{
	ax_btrie_r btrie = make_test_btrie();
	ax_list_r key = ax_new(ax_list, ax_trie_key_tr(btrie.ax_trie));

	ax_iter it, end;

	ax_box_clear(key.ax_box);

	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1));
	it = ax_trie_at(btrie.ax_trie, key.ax_seq);
	ut_assert_int_equal(r, 111, *(int*)ax_iter_get(&it));
	ut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 2));
	it = ax_trie_at(btrie.ax_trie, key.ax_seq);
	ut_assert_int_equal(r, 112, *(int*)ax_iter_get(&it));
	ut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1));
	it = ax_trie_at(btrie.ax_trie, key.ax_seq);
	ut_assert_int_equal(r, 11, *(int*)ax_iter_get(&it));
	ut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1));
	it = ax_trie_at(btrie.ax_trie, key.ax_seq);
	ut_assert_int_equal(r, 1, *(int*)ax_iter_get(&it));
	ut_assert(r, ax_trie_iter_valued(&it));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 2));
	it = ax_trie_at(btrie.ax_trie, key.ax_seq);
	end = ax_box_end(btrie.ax_box);
	ut_assert(r, !ax_iter_equal(&it, &end));
	ut_assert(r, !ax_trie_iter_valued(&it));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1, 1));
	it = ax_trie_at(btrie.ax_trie, key.ax_seq);
	end = ax_box_end(btrie.ax_box);
	ut_assert(r, ax_iter_equal(&it, &end));

	ax_one_free(btrie.ax_one);
	ax_one_free(key.ax_one);
}

static void trie_exist(ut_runner *r)
{


	ax_btrie_r btrie = make_test_btrie();
	ax_list_r key = ax_new(ax_list, ax_trie_key_tr(btrie.ax_trie));

	bool valued;

	ax_box_clear(key.ax_box);

	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1));
	ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);

	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 2));
	ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1));
	ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 2, 1));
	ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1));
	ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));


	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 2));
	ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, &valued));
	ut_assert(r, !valued);

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 2, 2));
	ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_one_free(btrie.ax_one);
	ax_one_free(key.ax_one);
}

static void trie_erase(ut_runner *r)
{
	ax_btrie_r btrie = make_test_btrie();
	ax_list_r key = ax_new(ax_list, ax_trie_key_tr(btrie.ax_trie));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1));
	ax_trie_erase(btrie.ax_trie, key.ax_seq);
	ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 2));
	ax_trie_erase(btrie.ax_trie, key.ax_seq);
	ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1));
	ax_trie_erase(btrie.ax_trie, key.ax_seq);
	ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 2, 1));
	ax_trie_erase(btrie.ax_trie, key.ax_seq);
	ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1));
	ax_trie_erase(btrie.ax_trie, key.ax_seq);
	ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_box_clear(key.ax_box);
	ax_trie_erase(btrie.ax_trie, key.ax_seq);
	ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

	ax_one_free(btrie.ax_one);
	ax_one_free(key.ax_one);
}

static void trie_prune(ut_runner *r)
{
	ax_btrie_r btrie = make_test_btrie();
	ax_list_r key = ax_new(ax_list, ax_trie_key_tr(btrie.ax_trie));
	{
		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1));
		ax_trie_prune(btrie.ax_trie, key.ax_seq);
		ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));
		ut_assert_uint_equal(r, 6, ax_box_size(btrie.ax_box));
		ax_one_free(btrie.ax_one);
	}

	{
		btrie = make_test_btrie();
		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 2));
		ax_trie_prune(btrie.ax_trie, key.ax_seq);
		ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));
		ut_assert_uint_equal(r, 6, ax_box_size(btrie.ax_box));
		ax_one_free(btrie.ax_one);
	}

	{
		btrie = make_test_btrie();
		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1));
		ax_trie_prune(btrie.ax_trie, key.ax_seq);
		ut_assert_uint_equal(r, 4, ax_box_size(btrie.ax_box));

		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1));
		ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 1));
		ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1, 1, 2));
		ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));
		ax_one_free(btrie.ax_one);
	}

	{
		btrie = make_test_btrie();
		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 1));
		ax_trie_prune(btrie.ax_trie, key.ax_seq);
		ut_assert_uint_equal(r, 2, ax_box_size(btrie.ax_box));

		ax_box_clear(key.ax_box);
		ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));

		ax_box_clear(key.ax_box);
		ax_seq_push_arraya(key.ax_seq, ax_arraya(int, 2, 1, 1));
		ut_assert(r, ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));
		ax_one_free(btrie.ax_one);
	}

	{
		btrie = make_test_btrie();
		ax_box_clear(key.ax_box);
		ax_trie_prune(btrie.ax_trie, key.ax_seq);
		ut_assert_uint_equal(r, 0, ax_box_size(btrie.ax_box));

		ax_box_clear(key.ax_box);
		ut_assert(r, !ax_trie_exist(btrie.ax_trie, key.ax_seq, NULL));
		ax_one_free(btrie.ax_one);
	}

	ax_one_free(key.ax_one);
}

static void trie_rekey(ut_runner *r)
{
	ut_assert(r, !"BUG");
}

ut_suite *suite_for_btrie()
{
	ut_suite *suite = ut_suite_create("btrie");

	ut_suite_add(suite, create, 0);
	ut_suite_add(suite, trie_put, 1);
	ut_suite_add(suite, trie_get, 1);
	ut_suite_add(suite, trie_exist, 2);
	ut_suite_add(suite, trie_at, 2);
	ut_suite_add(suite, trie_erase, 2);
	ut_suite_add(suite, trie_prune, 2);
	ut_suite_add(suite, trie_rekey, 2);
	ut_suite_add(suite, iterater, 2);

	return suite;
}
