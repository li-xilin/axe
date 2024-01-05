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

#ifndef AX_CLASS_H
#define AX_CLASS_H

#include "trick.h"
#include "debug.h"
#include "narg.h"

#define __AX_CLASS_ENTRY_STRUCT(name) struct AX_CATENATE(name, _st)
#define __AX_CLASS_TRAIT_STRUCT(name) struct AX_CATENATE(name, _trait_st)
#define __AX_CLASS_ENV_STRUCT(name) struct AX_CATENATE(name, _env_st)

#define __AX_CREATOR_NAME(name, _n) __AX_CATENATE_3(name, _new, _n)

#define ax_new0(name) \
	(name##_r) { .__ptr = __AX_CREATOR_NAME(name, )() }

#define ax_new(name, ...) \
	(name##_r) { .__ptr = __AX_CREATOR_NAME(name, AX_NARG(__VA_ARGS__))(__VA_ARGS__) }

#define __AX_CLASS_EXTERN_TRAIT(name) \
	const __AX_CLASS_TRAIT_STRUCT(ax_base_of(1, name)) AX_CATENATE(name, _tr)

#define ax_concrete_creator0(name) \
	__AX_CLASS_ENTRY_STRUCT(ax_base_of(1, name)) *__AX_CREATOR_NAME(name, )(void)

#define ax_concrete_creator(name, ...) \
	__AX_CLASS_ENTRY_STRUCT(ax_base_of(1, name)) *__AX_CREATOR_NAME(name, AX_NARG(__VA_ARGS__))(__VA_ARGS__)

#define ax_abstract_root_code_begin(name) \
	typedef __AX_CLASS_TRAIT_STRUCT(name)  AX_CATENATE(name, _trait); \
	__AX_CLASS_TRAIT_STRUCT(name) {\

#define ax_abstract_code_begin(name) \
	ax_abstract_root_code_begin(name) \
		const __AX_CLASS_TRAIT_STRUCT(ax_base_of(1, name)) ax_base_of(1, name);

#define ax_abstract_root_data_begin(name) \
	typedef __AX_CLASS_ENV_STRUCT(name) AX_CATENATE(name, _env); \
	__AX_CLASS_ENV_STRUCT(name) {

#define ax_abstract_data_begin(name) \
	ax_abstract_root_data_begin(name) \
		__AX_CLASS_ENV_STRUCT(ax_base_of(1, name)) ax_base_of(1, name);

#define ax_concrete_begin(name) \
	__AX_CLASS_ENTRY_STRUCT(name) { \
		__AX_CLASS_ENTRY_STRUCT(ax_base_of(1, name)) ax_base_of(1, name);

#ifndef ax_end
#define ax_end }
#endif

#define __ax_base_of(n, name) __AX_PREFIX_TO_##n(ax_baseof_, name)
#define ax_base_of(n, name) __ax_base_of(n, name)

#define __AX_CLASS_DECLARE_VAR(const, name) const struct AX_CATENATE(name, _st) *name; 
#define __AX_CLASS_ROLE_ITEM(n, const, name) __AX_CLASS_DECLARE_VAR(const, ax_base_of(n, name))

#define __AX_CLASS_ROLE(n, name) \
	typedef union \
	{ \
		const void *__ptr; \
		AX_PAVE_TO(n, __AX_CLASS_ROLE_ITEM, const, name) \
		__AX_CLASS_DECLARE_VAR(const,  name) \
	} AX_CATENATE(name, _cr); \
	\
	typedef union \
	{ \
		void *__ptr; \
		AX_PAVE_TO(n, __AX_CLASS_ROLE_ITEM, , name) \
		__AX_CLASS_DECLARE_VAR(, name) \
	} AX_CATENATE(name, _r)

#define __AX_CLASS_ABSTRACT(name) \
	__AX_CLASS_ENTRY_STRUCT(name) \
	{ \
		const __AX_CLASS_TRAIT_STRUCT(name) *const tr; \
		__AX_CLASS_ENV_STRUCT(name) env; \
	}

#define __AX_CLASS_NAME_ITEM(n, name) ax_stringy(ax_base_of(n, name)) "."
#define ax_class_name(n, name) AX_PAVE_TO(n, __AX_CLASS_NAME_ITEM, name) ax_stringy(name)

#define ax_abstract_declare(n, name) \
	__AX_CLASS_ROLE(n, name); \
	__AX_CLASS_ABSTRACT(name)

#define ax_class_trait(obj) (*((obj)->tr))
#define ax_class_data(obj) ((obj)->env)
#define ax_class_do0(obj, func) ax_class_trait(obj).func(obj)
#define ax_class_do(obj, func, ...) ax_class_trait(obj).func(obj, __VA_ARGS__)

#define ax_concrete_declare(n, name) \
	__AX_CLASS_ROLE(n, name)

#define ax_r(type, ptr) ((type##_r){ .type = ptr })
#define ax_cr(type, ptr) ((type##_cr){ .type = ptr })

#define ax_r_isnull(r) (!(r).__ptr)
#define AX_R_NULL { .__ptr = NULL }
#define AX_R_INIT(type, p) { .type = p }

#endif
