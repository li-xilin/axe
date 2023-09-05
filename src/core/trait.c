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

#include "ax/trait.h"
#include "ax/debug.h"
#include "ax/mem.h"
#include "ax/dump.h"
#include "ax/def.h"

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <inttypes.h>
#include <assert.h>
#include <limits.h>

#include "check.h"

#define TYPE_i8 int8_t
#define TYPE_i16 int16_t
#define TYPE_i32 int32_t
#define TYPE_i64 int64_t

#define TYPE_u8 uint8_t
#define TYPE_u16 uint16_t
#define TYPE_u32 uint32_t
#define TYPE_u64 uint64_t

/*
#define TYPE_char char
#define TYPE_short short
#define TYPE_int int
#define TYPE_long long
#define TYPE_llong long long

#define TYPE_uchar unsigned char
#define TYPE_ushort unsigned short
#define TYPE_uint unsigned int
#define TYPE_ulong unsigned long
#define TYPE_ullong unsigned long long
*/

#define TYPE_void void
#define TYPE_float float
#define TYPE_double double
#define TYPE_size size_t
#define TYPE_diff ptrdiff_t
#define TYPE_ptr void *
#define TYPE_str char *
#define TYPE_wcs wchar_t *

static void do_nothing()
{

}

static bool do_nothing_ret_false()
{
	return false;
}

static bool do_nothing_ret_true()
{
	return true;
}

static ax_dump *dump_void(const void* p)
{
	return ax_dump_symbol("nil");
}

static ax_dump *dump_i8(const void* p)
{
	return ax_dump_int(*(int8_t *)p);
}

static ax_dump *dump_i16(const void* p)
{
	return ax_dump_int(*(int16_t *)p);
}

static ax_dump *dump_i32(const void* p)
{
	return ax_dump_int(*(int32_t *)p);
}

static ax_dump *dump_i64(const void* p)
{
	return ax_dump_int(*(int64_t *)p);
}

static ax_dump *dump_u8(const void* p)
{
	return ax_dump_int(*(uint8_t *)p);
}

static ax_dump *dump_u16(const void* p)
{
	return ax_dump_int(*(uint16_t *)p);
}

static ax_dump *dump_u32(const void* p)
{
	return ax_dump_int(*(uint32_t *)p);
}

static ax_dump *dump_u64(const void* p)
{
	return ax_dump_int(*(uint64_t *)p);
}

static ax_dump *dump_float(const void* p)
{
	return ax_dump_float(*(float *)p);
}

static ax_dump *dump_double(const void* p)
{
	return ax_dump_float(*(double *)p);
}

static ax_dump *dump_size(const void* p)
{
	return ax_dump_uint(*(size_t *)p);
}

static ax_dump *dump_diff(const void* p)
{
	return ax_dump_int(*(ptrdiff_t *)p);
}

static ax_dump *dump_str(const void* p)
{
	return ax_dump_str(*(char **)p);
}

static ax_dump *dump_wcs(const void* p)
{
	return ax_dump_wcs(*(wchar_t**)p);
}

static ax_dump *dump_ptr(const void* p)
{
	return ax_dump_ptr(p);
}

// ------

#define TRAIT_EQUAL(name) \
	static bool equal_##name(const void* p1, const void* p2) \
	{ \
		return *(const TYPE_##name*) p1 == *(const TYPE_##name*) p2; \
	}

TRAIT_EQUAL(i8)
TRAIT_EQUAL(i16)
TRAIT_EQUAL(i32)
TRAIT_EQUAL(i64)
TRAIT_EQUAL(u8)
TRAIT_EQUAL(u16)
TRAIT_EQUAL(u32)
TRAIT_EQUAL(u64)
TRAIT_EQUAL(float)
TRAIT_EQUAL(double)
TRAIT_EQUAL(size)
TRAIT_EQUAL(diff)
TRAIT_EQUAL(ptr)

static bool equal_str(const void* p1, const void* p2)
{
	return strcmp(*(char**)p1, *(char**)p2) == 0;
}

static bool equal_wcs(const void* p1, const void* p2)
{
	return wcscmp(*(wchar_t**)p1, *(wchar_t**)p2) == 0;
}

// ------


#define TRAIT_LESS(name) \
	static bool less_##name(const void* p1, const void* p2) \
	{ \
		return *(const TYPE_##name*) p1 < *(const TYPE_##name*) p2; \
	}

TRAIT_LESS(i8)
TRAIT_LESS(i16)
TRAIT_LESS(i32)
TRAIT_LESS(i64)
TRAIT_LESS(u8)
TRAIT_LESS(u16)
TRAIT_LESS(u32)
TRAIT_LESS(u64)
TRAIT_LESS(float)
TRAIT_LESS(double)
TRAIT_LESS(size)
TRAIT_LESS(diff)
TRAIT_LESS(ptr)

static bool less_void(const void* p1, const void* p2)
{
	return false;
}

static bool less_str(const void* p1, const void* p2)
{
	return strcmp(*(char**)p1, *(char**)p2) < 0;
}

static bool less_wcs(const void* p1, const void* p2)
{
	return wcscmp(*(wchar_t**)p1, *(wchar_t**)p2) < 0;
}

// ------

#define TRAIT_COPY(name) \
	static ax_fail copy_##name(void* dst, const void* src) \
	{ \
		*(TYPE_##name *)dst = *(TYPE_##name *)src; \
		return false; \
	}

TRAIT_COPY(i8)
TRAIT_COPY(i16)
TRAIT_COPY(i32)
TRAIT_COPY(i64)
TRAIT_COPY(u8)
TRAIT_COPY(u16)
TRAIT_COPY(u32)
TRAIT_COPY(u64)
TRAIT_COPY(float)
TRAIT_COPY(double)
TRAIT_COPY(size)
TRAIT_COPY(diff)
TRAIT_COPY(ptr)

#define copy_void do_nothing_ret_false

static ax_fail copy_str(void* dst, const void* src)
{
	return !((*(char**)dst = ax_strdup(*(char**)src)));
}

static ax_fail copy_wcs(void* dst, const void* src)
{
	return !(*(wchar_t**)dst = ax_wcsdup(*(wchar_t**)src));
}

// ------

ax_fail init_i8(void* p, va_list *ap)
{
	*(int8_t *)p = ap ? va_arg(*ap, int) : 0;
	return false;
}

ax_fail init_i16(void* p, va_list *ap)
{
	*(int16_t *)p = ap ? va_arg(*ap, int) : 0;
	return false;
}

ax_fail init_i32(void* p, va_list *ap)
{
	*(int32_t *)p = ap ? va_arg(*ap, int32_t) : 0;
	return false;
}

ax_fail init_i64(void* p, va_list *ap)
{
	*(int64_t *)p = ap ? va_arg(*ap, uint64_t) : 0;
	return false;
}

ax_fail init_u8(void* p, va_list *ap)
{
	*(uint8_t *)p = ap ? va_arg(*ap, int) : 0;
	return false;
}

ax_fail init_u16(void* p, va_list *ap)
{
	*(uint16_t *)p = ap ? va_arg(*ap, int) : 0;
	return false;
}

ax_fail init_u32(void* p, va_list *ap)
{
	*(uint32_t *)p = ap ? va_arg(*ap, uint32_t) : 0;
	return false;
}

ax_fail init_u64(void* p, va_list *ap)
{
	*(uint64_t *)p = ap ? va_arg(*ap, uint64_t) : 0;
	return false;
}

ax_fail init_size(void* p, va_list *ap)
{
	*(size_t *)p = ap ? va_arg(*ap, size_t) : 0;
	return false;
}

ax_fail init_float(void* p, va_list *ap)
{
	*(float *)p = ap ? va_arg(*ap, double) : 0;
	return false;
}

ax_fail init_double(void* p, va_list *ap)
{
	*(double *)p = ap ? va_arg(*ap, double) : 0;
	return false;
}

ax_fail init_ptr(void* p, va_list *ap)
{
	*(void **)p = ap ? va_arg(*ap, void *) : NULL;
	return false;
}

ax_fail init_diff(void* p, va_list *ap)
{
	*(ptrdiff_t *)p = ap ? va_arg(*ap, ptrdiff_t) : 0;
	return false;
}

static ax_fail init_str(void* p, va_list *ap) {
	char *str = ap ? va_arg(*ap, void *) : "";
	size_t len = strlen(str);

	char *newstr = malloc(len + 1);
	if (!newstr)
		return true;
	memcpy(newstr, str, (len + 1) * sizeof(char));
	*(void**)  p = newstr;
	return false;
}

static ax_fail init_wcs(void* p, va_list *ap) {
	wchar_t *str = ap ? va_arg(*ap, void *) : "";
	size_t len = wcslen(str);

	wchar_t *newstr = malloc((len + 1) * sizeof(wchar_t));
	if (!newstr)
		return true;
	memcpy(newstr, str, (len + 1) * sizeof(wchar_t));
	*(void**)  p = newstr;
	return false;
}

// ------


// -----------------------------------------------------

#define TRAIT_HASH(name) \
	size_t hash_##name(const void* p) \
	{ \
		return ax_hash_djb(p, sizeof(TYPE_##name)); \
	}

TRAIT_HASH(i8)
TRAIT_HASH(i16)
TRAIT_HASH(i32)
TRAIT_HASH(i64)
TRAIT_HASH(u8)
TRAIT_HASH(u16)
TRAIT_HASH(u32)
TRAIT_HASH(u64)
TRAIT_HASH(float)
TRAIT_HASH(double)
TRAIT_HASH(size)
TRAIT_HASH(diff)
TRAIT_HASH(ptr)

static size_t hash_void(const void* p)
{
	return ax_hash_djb(NULL, 0);
} 

static size_t hash_str(const void* p)
{
	return ax_strhash(*(char**)p);
} 

static size_t hash_wcs(const void* p)
{
	return ax_wcshash(*(wchar_t**)p);
} 

// ------

static void free_str(void* p)
{
	free(*(char**)p);
}

static void free_wcs(void* p)
{
	free(*(char**)p);
}

const ax_trait ax_t_void = { 
	.t_size  = 0,
	.t_equal = do_nothing_ret_true,
	.t_less  = less_void,
	.t_dump  = dump_void,
	.t_hash  = hash_void,
	.t_free  = do_nothing,
	.t_copy  = copy_void,
	.t_init  = do_nothing_ret_false,
	.t_link  = false
};

const ax_trait ax_t_i8 = { 
	.t_size  = sizeof(int8_t),
	.t_equal = equal_i8,
	.t_less  = less_i8,
	.t_dump  = dump_i8,
	.t_hash  = hash_i8,
	.t_free  = do_nothing,
	.t_copy  = copy_i8,
	.t_init  = init_i8,
	.t_link  = false
};

const ax_trait ax_t_i16 = { 
	.t_size  = sizeof(int16_t),
	.t_equal = equal_i16,
	.t_less  = less_i16,
	.t_dump  = dump_i16,
	.t_hash  = hash_i16,
	.t_free  = do_nothing,
	.t_copy  = copy_i16,
	.t_init  = init_i16,
	.t_link  = false
};

const ax_trait ax_t_i32 = { 
	.t_size  = sizeof(int32_t),
	.t_equal = equal_i32,
	.t_less  = less_i32,
	.t_dump  = dump_i32,
	.t_hash  = hash_i32,
	.t_free  = do_nothing,
	.t_copy  = copy_i32,
	.t_init  = init_i32,
	.t_link  = false
};

const ax_trait ax_t_i64 = { 
	.t_size  = sizeof(int64_t),
	.t_equal = equal_i64,
	.t_less  = less_i64,
	.t_dump  = dump_i64,
	.t_hash  = hash_i64,
	.t_free  = do_nothing,
	.t_copy  = copy_i64,
	.t_init  = init_i64,
	.t_link  = false
};

const ax_trait ax_t_u8 = { 
	.t_size  = sizeof(uint8_t),
	.t_equal = equal_u8,
	.t_less  = less_u8,
	.t_dump  = dump_u8,
	.t_hash  = hash_u8,
	.t_free  = do_nothing,
	.t_copy  = copy_u8,
	.t_init  = init_u8,
	.t_link  = false
};

const ax_trait ax_t_u16 = { 
	.t_size  = sizeof(uint16_t),
	.t_equal = equal_u16,
	.t_less  = less_u16,
	.t_dump  = dump_u16,
	.t_hash  = hash_u16,
	.t_free  = do_nothing,
	.t_copy  = copy_u16,
	.t_init  = init_u16,
	.t_link  = false
};

const ax_trait ax_t_u32 = { 
	.t_size  = sizeof(uint32_t),
	.t_equal = equal_u32,
	.t_less  = less_u32,
	.t_dump  = dump_u32,
	.t_hash  = hash_u32,
	.t_free  = do_nothing,
	.t_copy  = copy_u32,
	.t_init  = init_u32,
	.t_link  = false
};

const ax_trait ax_t_u64 = { 
	.t_size  = sizeof(int64_t),
	.t_equal = equal_u64,
	.t_less  = less_u64,
	.t_dump  = dump_u64,
	.t_hash  = hash_u64,
	.t_free  = do_nothing,
	.t_copy  = copy_u64,
	.t_init  = init_u64,
	.t_link  = false
};

const ax_trait ax_t_size = { 
	.t_size  = sizeof(size_t),
	.t_equal = equal_size,
	.t_less  = less_size,
	.t_dump  = dump_size,
	.t_hash  = hash_size,
	.t_free  = do_nothing,
	.t_copy  = copy_size,
	.t_init  = init_size,
	.t_link  = false
};

const ax_trait ax_t_float = { 
	.t_size  = sizeof(float),
	.t_equal = equal_float,
	.t_less  = less_float,
	.t_dump  = dump_float,
	.t_hash  = hash_float,
	.t_free  = do_nothing,
	.t_copy  = copy_float,
	.t_init  = init_float,
	.t_link  = false
};

const ax_trait ax_t_double = { 
	.t_size  = sizeof(double),
	.t_equal = equal_double,
	.t_less  = less_double,
	.t_dump  = dump_double,
	.t_hash  = hash_double,
	.t_free  = do_nothing,
	.t_copy  = copy_double,
	.t_init  = init_double,
	.t_link  = false
};

const ax_trait ax_t_ptr= { 
	.t_size  = sizeof(void *),
	.t_equal = equal_ptr,
	.t_less  = less_ptr,
	.t_dump  = dump_ptr,
	.t_hash  = hash_ptr,
	.t_free  = do_nothing,
	.t_copy  = copy_ptr,
	.t_init  = init_ptr,
	.t_link  = false
};

const ax_trait ax_t_str = { 
	.t_size  = sizeof(void*),
	.t_equal = equal_str,
	.t_less  = less_str,
	.t_dump  = dump_str,
	.t_hash  = hash_str,
	.t_free  = free_str,
	.t_copy  = copy_str,
	.t_init  = init_str,
	.t_link  = true
};

const ax_trait ax_t_wcs = { 
	.t_size  = sizeof(void*),
	.t_equal = equal_wcs,
	.t_less  = less_wcs,
	.t_dump  = dump_wcs,
	.t_hash  = hash_wcs,
	.t_free  = free_wcs,
	.t_copy  = copy_wcs,
	.t_init  = init_wcs,
	.t_link  = true
};

const ax_trait ax_t_diff = { 
	.t_size  = sizeof(void*),
	.t_equal = equal_diff,
	.t_less  = less_diff,
	.t_dump  = dump_diff,
	.t_hash  = hash_diff,
	.t_free  = do_nothing,
	.t_copy  = copy_diff,
	.t_init  = init_diff,
	.t_link  = true
};

ax_dump *ax_t_dump(const ax_trait *tr, const void* p)
{
	return tr->t_dump ? tr->t_dump(p) : ax_dump_symbol("?");
}

