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
#include "flow.h"

#define ax_require(_one, _op) ( \
		ax_assert(_one, "NULL object is specified for ax_require"), \
		ax_assert((_op), \
			"operation for %s is required, but not implemented", \
			((ax_one *)(_one))->tr->name) \
)

#define ax_null { .one = NULL }

#ifndef AX_ONE_DEFINED
#define AX_ONE_DEFINED
typedef struct ax_one_st ax_one;
#endif

#define ax_scope(...) \
	for (ax_one *__ax_auto_vars[] = { (ax_one *)1, __VA_ARGS__ }; \
			__ax_auto_vars[0]; \
			ax_one_multi_free(__ax_auto_vars + 1, ax_nelems(__ax_auto_vars) - 1), \
				__ax_auto_vars[0] = NULL)

#define ax_baseof_one

typedef void (*ax_one_free_f) (ax_one *one);
typedef const char *(*ax_one_name_f) (const ax_one *one);

ax_begin_root_trait(one)
	const ax_one_name_f name;
	const ax_one_free_f free;
ax_end;

ax_begin_root_env(one)
	struct {
		ax_one *macro;
		uintptr_t micro;
	} scope;
ax_end;

ax_bless(0, one);

inline static const char *ax_one_name(const ax_one *one)
{
	ax_require(one, one->tr->name);
	return one->tr->name(one);
}

inline static void ax_one_free(ax_one *one)
{
	if (!one) 
		return;
	ax_require(one, one->tr->free);
	one->tr->free(one);
}

inline static ax_one_env *ax_one_envp(const ax_one *one)
{
	return (ax_one_env *)((char*)one + sizeof (ax_one_trait *));
}

bool ax_one_is(const ax_one *one, const char *type);

inline static void ax_one_multi_free(ax_one *one[], size_t count)
{
	ax_repeat(count)
		ax_one_free(one[_]);
}

#endif

