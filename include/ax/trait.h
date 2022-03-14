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
#include "debug.h"
#include <stdint.h>
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

#define __ax_require(_fun) ax_assert(_fun, "NULL pointer of %s", _fun)

#define ax_t(_name) (&ax_trait_##_name)

typedef void    (*ax_trait_free_f)(void* p);
typedef bool    (*ax_trait_compare_f) (const void* p1, const void* p2);
typedef size_t  (*ax_trait_hash_f)(const void* p);
typedef ax_fail (*ax_trait_copy_f)(void* dst, const void* src);
typedef ax_fail (*ax_trait_init_f)(void* p, va_list *ap);
typedef ax_dump*(*ax_trait_dump_f)(const void* p);

struct ax_trait_st
{
	const size_t       size;
	ax_trait_compare_f equal;
	ax_trait_compare_f less;
	ax_trait_hash_f    hash;
	ax_trait_free_f    free; 
	ax_trait_copy_f    copy;
	ax_trait_init_f    init;
	ax_trait_dump_f    dump;
	bool               link;
};


#define ax_trait_char ax_trait_i8
#define ax_trait_uchar ax_trait_u8

#if USHRT_MAX == UINT8_MAX
#define ax_trait_short ax_trait_i8
#define ax_trait_ushort ax_trait_u8
#elif USHRT_MAX == UINT16_MAX
#define ax_trait_short ax_trait_i16
#define ax_trait_ushort ax_trait_u16
#elif USHRT_MAX == UINT32_MAX
#define ax_trait_short ax_trait_i32
#define ax_trait_ushort ax_trait_u32
#else
#error "failed to match the size of short int"
#endif

#if UINT_MAX == UINT16_MAX
#define ax_trait_int ax_trait_i16
#define ax_trait_uint ax_trait_u16
#elif UINT_MAX == UINT32_MAX
#define ax_trait_int ax_trait_i32
#define ax_trait_uint ax_trait_u32
#elif UINT_MAX == INT64_MAX
#define ax_trait_int ax_trait_i64
#define ax_trait_uint ax_trait_u64
#else
#error "failed to match the size of int"
#endif

#if ULONG_MAX == UINT32_MAX
#define ax_trait_long ax_trait_i32
#define ax_trait_ulong ax_trait_u32
#elif ULONG_MAX == UINT64_MAX
#define ax_trait_long ax_trait_i64
#define ax_trait_ulong ax_trait_u64
#else
#error "failed to match the size of long int"
#endif

#if ULLONG_MAX == UINT32_MAX
#define ax_trait_llong ax_trait_i32
#define ax_trait_ullong ax_trait_u32
#elif ULLONG_MAX == UINT64_MAX
#define ax_trait_llong ax_trait_i64
#define ax_trait_ullong ax_trait_u64
#else
#error "failed to match the size of long long int"
#endif

extern const ax_trait ax_trait_void;
extern const ax_trait ax_trait_i8;
extern const ax_trait ax_trait_i16;
extern const ax_trait ax_trait_i32;
extern const ax_trait ax_trait_i64;
extern const ax_trait ax_trait_u8;
extern const ax_trait ax_trait_u16;
extern const ax_trait ax_trait_u32;
extern const ax_trait ax_trait_u64;
extern const ax_trait ax_trait_size;
extern const ax_trait ax_trait_float;
extern const ax_trait ax_trait_double;
extern const ax_trait ax_trait_ptr;
extern const ax_trait ax_trait_str;
extern const ax_trait ax_trait_wcs;
extern const ax_trait ax_trait_diff;

#define ax_trait_in(_tr, _ptr) ((_tr)->link ? &(_ptr) : *&(_ptr))

#define ax_trait_out(_tr, _ptr) (((_tr)->link && (_ptr)) ? *(void **)(_ptr) : (void *)(_ptr))

inline static bool ax_trait_equal(const ax_trait *tr, const void *p1, const void *p2)
{
	__ax_require(tr->equal);
	return tr->equal(p1, p2);
}

inline static size_t ax_trait_hash(const ax_trait *tr, const void *p)
{
	__ax_require(tr->hash);
	return tr->hash(p);
}

inline static void ax_trait_free(const ax_trait *tr, void *p)
{
	__ax_require(tr->free);
	if (tr->link && !*(void **)p)
		return;
	tr->free(p);
}

inline static bool ax_trait_less(const ax_trait *tr, const void *p1, const void *p2)
{
	__ax_require(tr->less);
	return tr->less(p1, p2);
}

inline static ax_fail ax_trait_copy(const ax_trait *tr, void* dst, const void* src)
{
	__ax_require(tr->copy);
	return tr->copy(dst, src);
}

inline static ax_fail ax_trait_init(const ax_trait *tr, void* p, va_list *ap)
{
	__ax_require(tr->init);
	return tr->init(p, ap);
}

inline static ax_fail ax_trait_copy_or_init(const ax_trait *tr, void* dst, const void *src, va_list *ap)
{
	return src ? ax_trait_copy(tr, dst, src)
		: ax_trait_init(tr, dst, ap);
}

ax_dump *ax_trait_dump(const ax_trait *tr, const void* p);

#undef __ax_require

#endif
