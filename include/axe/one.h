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

#ifndef AXE_ONE_H_
#define AXE_ONE_H_
#include "def.h"
#include "debug.h"

#define AX_ONE_NAME "one"

#define ax_trait_require(_one, _trait) ax_assert((_trait), \
		"operation for %s is required, do implement it", \
		((ax_one *)(_one))->tr->name)

#define ax_trait_optional(_one, _trait) ax_assert((_trait), \
		"operation for %s is not supported", \
		((ax_one *)(_one))->tr->name)

#ifndef AX_BASE_DEFINED
#define AX_BASE_DEFINED
typedef struct ax_base_st ax_base;
#endif

#ifndef AX_ONE_DEFINED
#define AX_ONE_DEFINED
typedef struct ax_one_st ax_one;
#endif

#ifndef AX_SCOPE_DEFINED
#define AX_SCOPE_DEFINED
typedef struct ax_scope_st ax_scope;
#endif

typedef struct ax_one_env_st ax_one_env;

typedef void        (*ax_one_free_f) (ax_one *one);
typedef ax_scope   *(*ax_one_scope_f)(const ax_one *one);
typedef ax_one_env *(*ax_one_envp_f) (const ax_one *one);
typedef ax_base    *(*ax_one_base_f)(const ax_one *one);

struct ax_one_trait_st
{
	const char          *name;
	const ax_one_free_f  free;
};
typedef struct ax_one_trait_st ax_one_trait;

typedef union
{
	ax_one *one;
} ax_one_cr;

typedef union
{
	ax_one *one;
	ax_one_cr c;
} ax_one_r;

struct ax_one_env_st
{
	ax_base *const base;
	ax_scope *scope;
	size_t sindex;
};

struct ax_one_st
{
	const ax_one_trait *const tr; /* Keep this first */
	ax_one_env env;
};

static inline const char *ax_one_name(const ax_one *one)
{
	ax_trait_require(one, one->tr->name);
	return one->tr->name;
}

static inline void ax_one_free(ax_one *one) {
	ax_trait_require(one, one->tr->free);
	one->tr->free(one);
}


static inline ax_one_env *ax_one_envp(const ax_one *one)
{
	return (ax_one_env *)((char*)one + sizeof (ax_one_trait *));
}

static inline ax_scope *ax_one_scope(const ax_one *one)
{
	return ax_one_envp(one)->scope;
}

static inline ax_base *ax_one_base(const ax_one *one)
{
	return ax_one_envp(one)->base;
}

ax_bool ax_one_is(const ax_one *one, const char *type);

#define ax_r(type, ptr) ((ax_##type##_r){ .type = ptr })
#define ax_cr(type, ptr) ((ax_##type##_cr){ .type = ptr })

#endif

