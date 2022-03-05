/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
 *
 * Permission is hereby granted, free of charge, to one person obtaining a copy
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

#ifndef AX_ONE_H
#define AX_ONE_H
#include "def.h"
#include "debug.h"
#include "class.h"

#define AX_ONE_NAME "one"

#define ax_require(_one, _op) (ax_assert(_one, "NULL object is specified for ax_require"), ax_assert((_op), \
		"operation for %s is required, but not implemented", \
		((ax_one *)(_one))->tr->name))

#define ax_null { .one = NULL }

#define AX_CLASS_ROLE_one(_l) _l struct ax_one_st *one;

#ifndef AX_ONE_DEFINED
#define AX_ONE_DEFINED
typedef struct ax_one_st ax_one;
#endif

typedef void (*ax_one_free_f) (ax_one *one);

typedef struct ax_one_trait_st
{
	const char          *name;
	const ax_one_free_f  free;
} ax_one_trait;

typedef struct ax_one_env_st
{
	struct {
		ax_one *macro;
		uintptr_t micro;
	} scope;
} ax_one_env;

AX_BLESS(one);

static inline const char *ax_one_name(const ax_one *one)
{
	ax_require(one, one->tr->name);
	return one->tr->name;
}

static inline void ax_one_free(ax_one *one) {
	if (!one) 
		return;
	ax_require(one, one->tr->free);
	one->tr->free(one);
}

static inline ax_one_env *ax_one_envp(const ax_one *one)
{
	return (ax_one_env *)((char*)one + sizeof (ax_one_trait *));
}

bool ax_one_is(const ax_one *one, const char *type);

#endif

