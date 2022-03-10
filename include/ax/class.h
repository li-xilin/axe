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

#include "def.h"
#include "macro.h"

#define __AX_CLASS_ENTRY_STRUCT(name) struct AX_CATENATE(ax_, name, _st)
#define __AX_CLASS_TRAIT_STRUCT(name) struct AX_CATENATE(ax_, name, _trait_st)
#define __AX_CLASS_ENV_STRUCT(name) struct AX_CATENATE(ax_, name, _env_st)

#define ax_class_new_n(_n, name, ...) \
	(ax_##name##_r) { .ax_base_of(1, name) = AX_CATENATE_4(__ax_, name, _construct, _n)(__VA_ARGS__) }

#define ax_new0(name) \
	ax_class_new_n(0, name, )

#define ax_new(name, ...) \
	ax_class_new_n(AX_NARG(__VA_ARGS__), name, __VA_ARGS__)

#define __AX_CLASS_EXTERN_TRAIT(name) \
	extern const __AX_CLASS_TRAIT_STRUCT(ax_base_of(1, name)) AX_CATENATE(ax_, name, _tr)

#define ax_class_constructor0(name) \
	__AX_CLASS_ENTRY_STRUCT(ax_base_of(1, name)) *AX_CATENATE(__ax_, name, _construct0)()

#define ax_class_constructor(name, ...) \
	__AX_CLASS_ENTRY_STRUCT(ax_base_of(1, name)) *AX_CATENATE(__ax_, name, _construct, AX_NARG(__VA_ARGS__))(__VA_ARGS__)

#define ax_begin_root_trait(name) \
	typedef __AX_CLASS_TRAIT_STRUCT(name)  AX_CATENATE(ax_, name, _trait); \
	__AX_CLASS_TRAIT_STRUCT(name) {\

#define ax_begin_trait(name) \
	ax_begin_root_trait(name) \
		const __AX_CLASS_TRAIT_STRUCT(ax_base_of(1, name)) ax_base_of(1, name);

#define ax_begin_root_env(name) \
	typedef __AX_CLASS_ENV_STRUCT(name) AX_CATENATE(ax_, name, _env); \
	__AX_CLASS_ENV_STRUCT(name) {

#define ax_begin_env(name) \
	ax_begin_root_env(name) \
		__AX_CLASS_ENV_STRUCT(ax_base_of(1, name)) ax_base_of(1, name);

#define ax_begin_entry(name) \
	__AX_CLASS_ENTRY_STRUCT(name) { \
		__AX_CLASS_ENTRY_STRUCT(ax_base_of(1, name)) ax_base_of(1, name);

#define ax_end }

#define __ax_base_of(n, name) __AX_MACRO_EXPAND_##n(ax_baseof_, name)
#define ax_base_of(n, name) __ax_base_of(n, name)

#define __AX_CLASS_DECLARE_VAR(const, name) const struct AX_CATENATE_3(ax_, name, _st) *name; 
#define __AX_CLASS_ROLE_ITEM(n, const, name) __AX_CLASS_DECLARE_VAR(const, ax_base_of(n, name))

#define ax_role(n, name) \
	typedef union \
	{ \
		AX_MACRO_PAVE(n, __AX_CLASS_ROLE_ITEM, const, name) \
		__AX_CLASS_DECLARE_VAR(const,  name) \
	} AX_CATENATE(ax_, name, _cr); \
	\
	typedef union \
	{ \
		AX_MACRO_PAVE(n, __AX_CLASS_ROLE_ITEM, , name) \
		__AX_CLASS_DECLARE_VAR(, name) \
	} AX_CATENATE(ax_, name, _r)

#define AX_CLASS_STRUCT_TMPL(name) \
	__AX_CLASS_ENTRY_STRUCT(name) \
	{ \
		const __AX_CLASS_TRAIT_STRUCT(name) *const tr; \
		__AX_CLASS_ENV_STRUCT(name) env; \
	}

#define __AX_CLASS_NAME_ITEM(n, name) ax_stringy(ax_base_of(n, name)) "."
#define ax_class_name(n, name) AX_MACRO_PAVE(n, __AX_CLASS_NAME_ITEM, name) ax_stringy(name)

#define ax_bless(n, name) \
	ax_role(n, name); \
	AX_CLASS_STRUCT_TMPL(name)

#define ax_r(type, ptr) ((ax_##type##_r){ .type = ptr })
#define ax_cr(type, ptr) ((ax_##type##_cr){ .type = ptr })

/* To define a new class
 *
 * #include "object0.h"
 * 
 * #define AX_OBJECT1_NAME AX_OBJECT0_NAME ".object1"
 * 
 * #ifndef AX_LIST_DEFINED
 * #define AX_LIST_DEFINED
 * typedef struct ax_object1_st ax_object1;
 * #endif
 * 
 * #define AX_CLASS_BASE_object1 object0
 * #define AX_CLASS_ROLE_object1(_l) _l AX_CLASS_PTR(object1); AX_CLASS_ROLE_object0(_l)
 *
 * AX_BEGIN_TRAIT(object1)
 *     ...
 * AX_END;
 * 
 * AX_BEGIN_ENV(object1)
 *     ...
 * AX_END;
 * 
 * AX_BLESS(object1);
 * 
 */

#endif
