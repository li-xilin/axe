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
#include "../def.h"
#include "../debug.h"
#include "../class.h"
#include "../flow.h"

#define ax_obj_require(_obj, _op) \
( \
	ax_assert((_obj), "NULL object is specified"), \
	ax_assert( \
		ax_class_trait(_obj)._op, \
		"operation for %s is required, but not implemented", \
		ax_class_trait((ax_one *)(_obj)).name((ax_one *)(_obj)) \
	)  \
)

#define ax_obj_do(_obj, _op, ...) \
( \
	ax_obj_require(_obj, _op), \
	ax_class_do(_obj, _op, __VA_ARGS__) \
)

#define ax_obj_do0(_obj, _op) \
( \
	ax_obj_require(_obj, _op), \
	ax_class_do0(_obj, _op) \
)

#ifndef AX_ONE_DEFINED
#define AX_ONE_DEFINED
typedef struct ax_one_st ax_one;
#endif

#define ax_scope(...) \
	for (ax_one *__ax_scope_vars[] = { (ax_one *)1, __VA_ARGS__ }; \
			__ax_scope_vars[0]; \
			ax_one_multi_free(__ax_scope_vars + 1, ax_nelems(__ax_scope_vars) - 1), \
				__ax_scope_vars[0] = 0) \
		for (int __ax_scope_cont = 1; __ax_scope_cont; __ax_scope_cont = 0) /* clean memory even if break is called */



#define ax_baseof_ax_one

typedef void (*ax_one_free_f) (ax_one *one);
typedef const char *(*ax_one_name_f) (const ax_one *one);

ax_abstract_root_code_begin(ax_one)
	const ax_one_name_f name;
	const ax_one_free_f free;
ax_end;

ax_abstract_root_data_begin(ax_one)
	struct {
		ax_one *macro;
		uintptr_t micro;
	} scope;
ax_end;

ax_abstract_declare(0, ax_one);

inline static const char *ax_one_name(const ax_one *one)
{
	return ax_obj_do0(one, name);

}

inline static void ax_one_free(ax_one *one)
{
	if (!one) 
		return;
	ax_obj_do0(one, free);
}

bool ax_one_is(const ax_one *one, const char *type);

inline static void ax_one_multi_free(ax_one *one[], size_t count)
{
	ax_repeat(count)
		ax_one_free(one[_]);
}

#endif

