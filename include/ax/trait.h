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

#ifndef AX_TRAIT_H
#define AX_TRAIT_H
#include "def.h"
#include "mem.h"
#include "debug.h"
#include "trick.h"
#include <stdint.h>
#include <string.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#ifndef AX_TRAIT_DEFINED
#define AX_TRAIT_DEFINED
typedef struct ax_trait_st ax_trait;
#endif

#ifndef AX_DUMP_DEFINED
#define AX_DUMP_DEFINED
typedef struct ax_dump_st ax_dump;
#endif

#ifdef ax_end
#undef ax_end
#endif
#define ax_end }

typedef void    (*ax_trait_free_f)(void* p);
typedef bool    (*ax_trait_compare_f) (const void* p1, const void* p2);
typedef size_t  (*ax_trait_hash_f)(const void* p);
typedef ax_fail (*ax_trait_copy_f)(void* dst, const void* src);
typedef ax_fail (*ax_trait_init_f)(void* p, va_list *ap);
typedef ax_dump*(*ax_trait_dump_f)(const void* p);

struct ax_trait_st
{
	const size_t       t_size;
	ax_trait_compare_f t_equal;
	ax_trait_compare_f t_less;
	ax_trait_hash_f    t_hash;
	ax_trait_free_f    t_free; 
	ax_trait_copy_f    t_copy;
	ax_trait_init_f    t_init;
	ax_trait_dump_f    t_dump;
	bool               t_link;
};


#define __AX_TRAIT_STRUCT_NAME(_name) ax_t_##_name

#define ax_t(_name) (&__AX_TRAIT_STRUCT_NAME(_name))

#define ax_t_char ax_t_i8
#define ax_t_uchar ax_t_u8

#if USHRT_MAX == UINT8_MAX
#define ax_t_short ax_t_i8
#define ax_t_ushort ax_t_u8
#elif USHRT_MAX == UINT16_MAX
#define ax_t_short ax_t_i16
#define ax_t_ushort ax_t_u16
#elif USHRT_MAX == UINT32_MAX
#define ax_t_short ax_t_i32
#define ax_t_ushort ax_t_u32
#else
#error "failed to match the size of short int"
#endif

#if UINT_MAX == UINT16_MAX
#define ax_t_int ax_t_i16
#define ax_t_uint ax_t_u16
#elif UINT_MAX == UINT32_MAX
#define ax_t_int ax_t_i32
#define ax_t_uint ax_t_u32
#elif UINT_MAX == INT64_MAX
#define ax_t_int ax_t_i64
#define ax_t_uint ax_t_u64
#else
#error "failed to match the size of int"
#endif

#if ULONG_MAX == UINT32_MAX
#define ax_t_long ax_t_i32
#define ax_t_ulong ax_t_u32
#elif ULONG_MAX == UINT64_MAX
#define ax_t_long ax_t_i64
#define ax_t_ulong ax_t_u64
#else
#error "failed to match the size of long int"
#endif

#if ULLONG_MAX == UINT32_MAX
#define ax_t_llong ax_t_i32
#define ax_t_ullong ax_t_u32
#elif ULLONG_MAX == UINT64_MAX
#define ax_t_llong ax_t_i64
#define ax_t_ullong ax_t_u64
#else
#error "failed to match the size of long long int"
#endif

extern const ax_trait ax_t_void;
extern const ax_trait ax_t_i8;
extern const ax_trait ax_t_i16;
extern const ax_trait ax_t_i32;
extern const ax_trait ax_t_i64;
extern const ax_trait ax_t_u8;
extern const ax_trait ax_t_u16;
extern const ax_trait ax_t_u32;
extern const ax_trait ax_t_u64;
extern const ax_trait ax_t_size;
extern const ax_trait ax_t_float;
extern const ax_trait ax_t_double;
extern const ax_trait ax_t_ptr;
extern const ax_trait ax_t_str;
extern const ax_trait ax_t_wcs;
extern const ax_trait ax_t_diff;

#define ax_type(_name) ax_trait_type_##_name

typedef void      ax_type(void);
typedef int8_t    ax_type(i8);
typedef int16_t   ax_type(i16);
typedef int32_t   ax_type(int32);
typedef int64_t   ax_type(int64);
typedef uint8_t   ax_type(u8);
typedef uint16_t  ax_type(u16);
typedef uint32_t  ax_type(u32);
typedef uint64_t  ax_type(u64);
typedef size_t    ax_type(size);
typedef float     ax_type(float);
typedef double    ax_type(double);
typedef void*     ax_type(ptr);
typedef char*     ax_type(str);
typedef wchar_t*  ax_type(wcs);
typedef ptrdiff_t ax_type(diff);

#define __AX_TRAIT_SET_EQUAL(_func) .t_equal = _func
#define __AX_TRAIT_SET_LESS(_func) .t_less = _func
#define __AX_TRAIT_SET_HASH(_func) .t_hash = _func
#define __AX_TRAIT_SET_FREE(_func) .t_free = _func
#define __AX_TRAIT_SET_COPY(_func) .t_copy = _func
#define __AX_TRAIT_SET_INIT(_func) .t_init = _func
#define __AX_TRAIT_SET_DUMP(_func) .t_dump = _func
#define __AX_TRAIT_SET_LINK(_bool) .t_link = _bool

#define __AX_TRAIT_SET(i, x) __AX_TRAIT_SET_##x,

#define ax_trait_declare(_name, _type) \
	typedef _type ax_type(_name); \
	extern const struct ax_trait_st __AX_TRAIT_STRUCT_NAME(_name)

#define ax_trait_define(_name, ...) \
	const struct ax_trait_st __AX_TRAIT_STRUCT_NAME(_name) = \
	{ .t_size = sizeof(ax_type(_name)), AX_MTOOL_TRANSFORM(__AX_TRAIT_SET, __VA_ARGS__) }

#define ax_trait_define0(_name) \
	const struct ax_trait_st __AX_TRAIT_STRUCT_NAME(_name) = \
	{ .t_size = sizeof(ax_type(_name)) }

#define ax_trait_in(_tr, _ptr) ((_tr)->t_link ? &(_ptr) : *&(_ptr))

#define ax_trait_out(_tr, _ptr) (((_tr)->t_link && (_ptr)) ? *(void **)(_ptr) : (void *)(_ptr))

inline static bool ax_trait_equal(const ax_trait *tr, const void *p1, const void *p2)
{
	if(tr->t_equal)
		return tr->t_equal(p1, p2);
	return memcmp(p1, p2, tr->t_size) == 0;
}

inline static size_t ax_trait_hash(const ax_trait *tr, const void *p)
{
	if (tr->t_hash)
		return tr->t_hash(p);
	return ax_memhash(p, tr->t_size);
}

inline static void ax_trait_free(const ax_trait *tr, void *p)
{
	if (tr->t_link && !*(void **)p)
		return;
	if (!tr->t_free)
		return;
	tr->t_free(p);
}

inline static bool ax_trait_less(const ax_trait *tr, const void *p1, const void *p2)
{
	if (tr->t_less)
		return tr->t_less(p1, p2);
	return memcmp(p1, p2, tr->t_size) < 0;
}

inline static ax_fail ax_trait_copy(const ax_trait *tr, void* dst, const void* src)
{
	if (tr->t_copy)
		return tr->t_copy(dst, src);
	memcpy(dst, src, tr->t_size);
	return false;
}

inline static ax_fail ax_trait_init(const ax_trait *tr, void* p, va_list *ap)
{
	if (tr->t_init)
		return tr->t_init(p, ap);
	memset(p, 0, tr->t_size);
	return false;
}

inline static ax_fail ax_trait_copy_or_init(const ax_trait *tr, void* dst, const void *src, va_list *ap)
{
	return src ? ax_trait_copy(tr, dst, src)
		: ax_trait_init(tr, dst, ap);
}

inline static size_t ax_trait_size(const ax_trait *tr)
{
	return tr->t_size;
}

ax_dump *ax_trait_dump(const ax_trait *tr, const void* p);

#endif
