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
#include <axe/scope.h>

#include <axe/base.h>
#include <axe/pool.h>
#include <axe/one.h>
#include "check.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#undef free

struct ax_scope_st
{
	ax_one _one;
	ax_one **tab;
	size_t tab_size;
	size_t tab_capacity;
};

static void one_free(ax_one *one);

static void one_free(ax_one *one)
{
	if (!one)
		return;

	ax_scope_r role = { one };

	ax_scope_detach(one);
	while (role.scope->tab_size) {
		ax_one *elem = role.scope->tab[0];
		elem->tr->free(elem);
	}
	free(role.scope->tab);
	ax_pool_free(role.scope);
}

static const ax_one_trait one_trait = {
	.name = AX_SCOPE_NAME,
	.free = one_free
};

ax_one *__ax_scope_construct(ax_base *base)
{
	CHECK_PARAM_NULL(base);

	ax_pool *pool = ax_base_pool(base);
	ax_scope *scope = ax_pool_alloc(pool, sizeof(ax_scope));
	if (scope == NULL)
		return NULL;

	ax_scope scope_init = {
		._one = {
			.tr = &one_trait,
			.env = {
				.base = base,
				.scope = { NULL },
			},
		},
		.tab = NULL,
		.tab_size = 0,
		.tab_capacity = 0
	};
	memcpy(scope, &scope_init, sizeof scope_init);
	return &scope->_one;
}

ax_scope_r ax_scope_create(ax_scope *scope)
{
	CHECK_PARAM_NULL(scope);

	assert(scope);
	ax_base *base = ax_one_base(&scope->_one);
	ax_scope_r role = { .one = __ax_scope_construct(base) };
	if (role.one == NULL)
		return role;
	ax_scope_attach(scope, role.one);
	return role;
}

void ax_scope_attach(ax_scope *scope_attach, ax_one *one)
{
	CHECK_PARAM_NULL(scope_attach);
	CHECK_PARAM_NULL(one);
	/* TODO: check first 4 bytes(base pointer) that if it equals to base which scope is created from */

	ax_assert(ax_one_envp(one)->scope && ax_one_is(ax_one_envp(one)->scope, AX_SCOPE_NAME), "object can not be detached");
	
	if (ax_one_envp(one)->scope.macro == ax_r(scope, scope_attach).one)
		return;
	if (ax_one_envp(one)->scope.macro != NULL)
		ax_scope_detach(one);
	if (scope_attach->tab_size == scope_attach->tab_capacity) {
		scope_attach->tab_capacity <<= 1;
		scope_attach->tab_capacity |= 1;
		scope_attach->tab = realloc(scope_attach->tab, scope_attach->tab_capacity * sizeof(*scope_attach->tab));
	}
	scope_attach->tab[scope_attach->tab_size] = one;
	ax_one_envp(one)->scope.macro = ax_r(scope, scope_attach).one;
	ax_one_envp(one)->scope.micro = scope_attach->tab_size;
	scope_attach->tab_size ++;
}

void ax_scope_clean(ax_scope *scope)
{
	ax_scope_r scope_r = { .scope = scope };

	while (scope_r.scope->tab_size) {
		ax_one *elem = scope_r.scope->tab[0];
		elem->tr->free(elem);
	}

	scope_r.scope->tab_size = 0;
}

void ax_scope_detach(ax_one *one)
{
	CHECK_PARAM_NULL(one);

	ax_one_env *envp_detach = ax_one_envp(one);
	if (envp_detach->scope.macro == NULL)
		return;
	ax_assert(ax_one_envp(one)->scope && ax_one_is(ax_one_envp(one)->scope, AX_SCOPE_NAME), "object can not be detached");

	ax_scope * scope = (ax_scope *)envp_detach->scope.macro;

	assert(scope->tab[envp_detach->scope.micro] == one);
	ax_one *last = scope->tab[envp_detach->scope.micro] = scope->tab[--scope->tab_size];
	ax_one_env *last_envp = ax_one_envp(last);
	last_envp->scope.micro = envp_detach->scope.micro;
	envp_detach->scope.macro = NULL;
	envp_detach->scope.micro = 0;
}

void ax_scope_destroy(ax_scope *scope)
{
	CHECK_PARAM_NULL(scope);
	ax_one_free(&scope->_one);

}

