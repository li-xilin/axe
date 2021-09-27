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

#define __AX_CLASS_ENTRY_STRUCT(_name) struct AX_CATENATE(ax_, _name, _st)

#define ax_class_new0(_name) \
	(ax_##_name##_r) { .ax_base_of(_name) = AX_CATENATE(__ax_, _name, _construct0)() }

#define ax_class_new(_name, ...) \
	(ax_##_name##_r) { .ax_base_of(_name) = AX_CATENATE(__ax_, _name, _construct, AX_NARG(__VA_ARGS__))(__VA_ARGS__) }

#define AX_CLASS_CONSTRUCTOR0(_name) \
	__AX_CLASS_ENTRY_STRUCT(ax_base_of(_name)) *AX_CATENATE(__ax_, _name, _construct0)()

#define AX_CLASS_CONSTRUCTOR(_name, ...) \
	__AX_CLASS_ENTRY_STRUCT(ax_base_of(_name)) *AX_CATENATE(__ax_, _name, _construct, AX_NARG(__VA_ARGS__))(__VA_ARGS__)

#define AX_CLASS_STRUCT_TRAIT(_name) \
	typedef struct ax_##_name##_trait_st  ax_##_name##_trait; \
	struct ax_##_name##_trait_st \

#define AX_BEGIN_TRAIT(_name) \
	AX_CLASS_STRUCT_TRAIT(_name) { \
		const struct AX_CATENATE(ax_, ax_base_of(_name), _trait_st) ax_base_of(_name);

#define AX_END }

#define ax_base_of(_name) AX_CLASS_BASE_##_name

#define __AX_CLASS_ENV_STRUCT(_name) struct AX_CATENATE(ax_, _name, _env_st)


#define AX_CLASS_STRUCT_ENV(_name) \
	typedef __AX_CLASS_ENV_STRUCT(_name) AX_CATENATE(ax_, _name, _env); \
	__AX_CLASS_ENV_STRUCT(_name)

#define AX_BEGIN_ENV(_name) \
	AX_CLASS_STRUCT_ENV(_name) { \
		struct AX_CATENATE(ax_, ax_base_of(_name), _env_st) ax_base_of(_name);

#define AX_CLASS_STRUCT_ROLE(_name) \
	typedef union \
	{ \
		AX_CLASS_ROLE_##_name(const) \
	} AX_CATENATE(ax_, _name, _cr); \
	typedef union \
	{ \
		AX_CLASS_ROLE_##_name() \
	} AX_CATENATE(ax_, _name, _r) \

#define AX_CLASS_STRUCT_ENTRY(_name) \
	struct ax_##_name##_st \
	{ \
		struct AX_CATENATE(ax_, ax_base_of(_name), _st) ax_base_of(_name);

#define AX_CLASS_STRUCT_TMPL(_name) \
	struct ax_##_name##_st \
	{ \
		const struct AX_CATENATE(ax_, _name, _trait_st) *const tr; \
		struct AX_CATENATE(ax_, _name, _env_st) env; \
	}

#define AX_BLESS(_name) \
	AX_CLASS_STRUCT_ROLE(_name); \
	AX_CLASS_STRUCT_TMPL(_name)

#define AX_CLASS_PTR(_name) struct AX_CATENATE(ax_, _name, _st) *(_name)

#endif

