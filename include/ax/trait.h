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
enum {
	AX_ST_NIL,
	AX_ST_I8,
	AX_ST_I16,
	AX_ST_I32,
	AX_ST_I64,
	AX_ST_U8,
	AX_ST_U16,
	AX_ST_U32,
	AX_ST_U64,
	AX_ST_Z,
	AX_ST_F,
	AX_ST_LF,
	AX_ST_S,
	AX_ST_WS,
	AX_ST_PTR,

	AX_ST_C,
	AX_ST_H,
	AX_ST_I,
	AX_ST_L,
	AX_ST_LL,

	AX_ST_UC,
	AX_ST_UH,
	AX_ST_U,
	AX_ST_UL,
	AX_ST_ULL,
	AX_ST_PWL,
};

#define __AX_ST_nil    AX_ST_NIL
#define __AX_ST_i8     AX_ST_I8 
#define __AX_ST_i16    AX_ST_I16
#define __AX_ST_i32    AX_ST_I32
#define __AX_ST_i64    AX_ST_I64
#define __AX_ST_u8     AX_ST_U8 
#define __AX_ST_u16    AX_ST_U16
#define __AX_ST_u32    AX_ST_U32
#define __AX_ST_u64    AX_ST_U64
#define __AX_ST_size   AX_ST_Z
#define __AX_ST_float  AX_ST_F
#define __AX_ST_double AX_ST_LF
#define __AX_ST_str    AX_ST_S
#define __AX_ST_wcs    AX_ST_WS
#define __AX_ST_ptr    AX_ST_PTR
#define __AX_ST_char   AX_ST_C
#define __AX_ST_short  AX_ST_H
#define __AX_ST_int    AX_ST_I
#define __AX_ST_long   AX_ST_L
#define __AX_ST_llong  AX_ST_LL
#define __AX_ST_uchar  AX_ST_UC
#define __AX_ST_ushort AX_ST_UH
#define __AX_ST_uint   AX_ST_U
#define __AX_ST_ulong  AX_ST_UL
#define __AX_ST_ullong AX_ST_ULL

#define ax_t(_name) ax_trait_get(__AX_ST_##_name)

typedef void    (*ax_trait_free_f)(void* p);
typedef bool    (*ax_trait_compare_f) (const void* p1, const void* p2, size_t size);
typedef size_t  (*ax_trait_hash_f)(const void* p, size_t size);
typedef ax_fail (*ax_trait_copy_f)(void* dst, const void* src, size_t size);
typedef ax_fail (*ax_trait_init_f)(void* p, va_list *ap);
typedef ax_dump*(*ax_trait_dump_f)(const void* p, size_t size);

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

#define ax_trait_in(_tr, _ptr) ((_tr)->link ? &(_ptr) : *&(_ptr))

#define ax_trait_out(_tr, _ptr) (((_tr)->link && (_ptr)) ? *(void **)(_ptr) : (void *)(_ptr))

inline static bool ax_trait_equal(const ax_trait *tr, const void *p1, const void *p2)
{
	__ax_require(tr->equal);
	return tr->equal(p1, p2, tr->size);
}

inline static size_t ax_trait_hash(const ax_trait *tr, const void *p)
{
	__ax_require(tr->hash);
	return tr->hash(p, tr->size);
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
	return tr->less(p1, p2, tr->size);
}

inline static ax_fail ax_trait_copy(const ax_trait *tr, void* dst, const void* src)
{
	__ax_require(tr->copy);
	return tr->copy(dst, src, tr->size);
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

ax_dump *ax_trait_dump(const ax_trait *tr, const void* p, size_t size);

bool ax_trait_mem_equal(const void* p1, const void* p2, size_t size);

bool ax_trait_mem_less(const void* p1, const void* p2, size_t size);

size_t  ax_trait_mem_hash(const void* p, size_t size);

ax_fail ax_trait_mem_copy(void* dst, const void* src, size_t size);

void  ax_trait_mem_free(void* p);

ax_dump *ax_trait_mem_dump(const void* p, size_t size);

size_t ax_trait_size(int type);

int ax_trait_fixed_type(int type);

const ax_trait *ax_trait_get(int type);

//int ax_trait_stoi(const char *s);

#undef __ax_require

#endif
