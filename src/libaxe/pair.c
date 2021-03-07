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

#include <axe/pair.h>
#include <axe/base.h>
#include <axe/pool.h>
#include <axe/scope.h>
#include <axe/debug.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "check.h"

struct ax_pair_st
{
	ax_one __one;
	ax_one_env one_env;
	const void *key;
	void *value;
};

static void one_free(ax_one *one);

static const ax_one_trait one_trait;

static void one_free(ax_one* one)
{
	if (!one)
		return;
	ax_scope_detach(one);
	ax_pool_free(one);
}

static const ax_one_trait one_trait =
{
	.name = "one:pair",
	.free = one_free,
	.envp = offsetof(ax_pair, one_env) 
};

ax_pair *__ax_pair_construct(ax_base *base, const void *key, void *value)
{
	CHECK_PARAM_NULL(base);

	ax_pool *pool = ax_base_pool(base);
	ax_pair *pair = ax_pool_alloc(pool, sizeof(ax_pair));
	ax_pair pair_init = {
		.__one = {
			.base = base,
			.tr = &one_trait,

		},
		.one_env = {
			.scope = NULL,
			.sindex = 0
		},
		.key = key,
		.value = value
	};
	memcpy(pair, &pair_init, sizeof pair_init);
	return pair;
}

ax_pair_role ax_pair_create(ax_scope* scope, const void *key, void *value)
{
	CHECK_PARAM_NULL(scope);
	ax_base *base = ax_one_base(ax_cast(scope, scope).one);
	ax_pair_role pair_r = { __ax_pair_construct(base, key, value) };
	if (pair_r.one == NULL)
		return pair_r;
	ax_scope_attach(scope, pair_r.one);
	return pair_r;
}

const void *ax_pair_key(ax_pair *pair)
{
	CHECK_PARAM_NULL(pair);

	return pair->key;
}

void *ax_pair_value(ax_pair *pair)
{
	CHECK_PARAM_NULL(pair);

	return pair->value;
}
