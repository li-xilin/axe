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

#include <ax/any.h>
#include <ax/stuff.h>
#include <ax/debug.h>
#include <ax/def.h>

#include <stdio.h>
#include <stdlib.h>

#include "check.h"

#undef free

static bool stuff_equal(const void* p1, const void* p2, size_t size)
{
	CHECK_PARAM_NULL(p1);
	CHECK_PARAM_NULL(p2);

	return (void*)p1 == (void*)p2;
}

static bool stuff_less(const void* p1, const void* p2, size_t size)
{
	CHECK_PARAM_NULL(p1);
	CHECK_PARAM_NULL(p2);

	return(void*)p1 < (void*)p2;
}

static void stuff_free(void* p)
{
	CHECK_PARAM_NULL(p);

	ax_one* one= (ax_one*)p;
	one->tr->free(one);
}

static size_t stuff_hash(const void* p, size_t size)
{
	CHECK_PARAM_NULL(p);

	return ax_stuff_traits(AX_ST_PTR)->hash(p, size);
}

static ax_fail stuff_copy(ax_pool* pool, void* dst, const void* src, size_t size)
{
	CHECK_PARAM_NULL(dst);
	CHECK_PARAM_NULL(src);

	ax_any* anys = *(ax_any**)src;
	return !(*(void**)dst = anys->tr->copy(src));
}

static bool stuff_init(ax_pool* pool, void *p, size_t size)
{
	CHECK_PARAM_NULL(pool);
	CHECK_PARAM_NULL(p);

	*(void**)p = NULL;
	return true;
}

static const ax_stuff_trait any_stuff_trait = {
	.size = sizeof(void*),
	.equal = stuff_equal,
	.less = stuff_less,
	.hash = stuff_hash,
	.free = stuff_free,
	.copy = stuff_copy,
	.init = stuff_init,
	.link = false

};

const ax_stuff_trait* __ax_any_stuff_trait()
{
	return &any_stuff_trait;
}

