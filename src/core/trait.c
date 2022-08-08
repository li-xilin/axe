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

const ax_trait ax_trait_void = { 
	.size  = 0,
	.equal = do_nothing_ret_true,
	.less  = less_void,
	.dump  = dump_void,
	.hash  = hash_void,
	.free  = do_nothing,
	.copy  = copy_void,
	.init  = do_nothing_ret_false,
	.link  = false
};

const ax_trait ax_trait_i8 = { 
	.size  = sizeof(int8_t),
	.equal = equal_i8,
	.less  = less_i8,
	.dump  = dump_i8,
	.hash  = hash_i8,
	.free  = do_nothing,
	.copy  = copy_i8,
	.init  = init_i8,
	.link  = false
};

const ax_trait ax_trait_i16 = { 
	.size  = sizeof(int16_t),
	.equal = equal_i16,
	.less  = less_i16,
	.dump  = dump_i16,
	.hash  = hash_i16,
	.free  = do_nothing,
	.copy  = copy_i16,
	.init  = init_i16,
	.link  = false
};

const ax_trait ax_trait_i32 = { 
	.size  = sizeof(int32_t),
	.equal = equal_i32,
	.less  = less_i32,
	.dump  = dump_i32,
	.hash  = hash_i32,
	.free  = do_nothing,
	.copy  = copy_i32,
	.init  = init_i32,
	.link  = false
};

const ax_trait ax_trait_i64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_i64,
	.less  = less_i64,
	.dump  = dump_i64,
	.hash  = hash_i64,
	.free  = do_nothing,
	.copy  = copy_i64,
	.init  = init_i64,
	.link  = false
};

const ax_trait ax_trait_u8 = { 
	.size  = sizeof(uint8_t),
	.equal = equal_u8,
	.less  = less_u8,
	.dump  = dump_u8,
	.hash  = hash_u8,
	.free  = do_nothing,
	.copy  = copy_u8,
	.init  = init_u8,
	.link  = false
};

const ax_trait ax_trait_u16 = { 
	.size  = sizeof(uint16_t),
	.equal = equal_u16,
	.less  = less_u16,
	.dump  = dump_u16,
	.hash  = hash_u16,
	.free  = do_nothing,
	.copy  = copy_u16,
	.init  = init_u16,
	.link  = false
};

const ax_trait ax_trait_u32 = { 
	.size  = sizeof(uint32_t),
	.equal = equal_u32,
	.less  = less_u32,
	.dump  = dump_u32,
	.hash  = hash_u32,
	.free  = do_nothing,
	.copy  = copy_u32,
	.init  = init_u32,
	.link  = false
};

const ax_trait ax_trait_u64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_u64,
	.less  = less_u64,
	.dump  = dump_u64,
	.hash  = hash_u64,
	.free  = do_nothing,
	.copy  = copy_u64,
	.init  = init_u64,
	.link  = false
};

const ax_trait ax_trait_size = { 
	.size  = sizeof(size_t),
	.equal = equal_size,
	.less  = less_size,
	.dump  = dump_size,
	.hash  = hash_size,
	.free  = do_nothing,
	.copy  = copy_size,
	.init  = init_size,
	.link  = false
};

const ax_trait ax_trait_float = { 
	.size  = sizeof(float),
	.equal = equal_float,
	.less  = less_float,
	.dump  = dump_float,
	.hash  = hash_float,
	.free  = do_nothing,
	.copy  = copy_float,
	.init  = init_float,
	.link  = false
};

const ax_trait ax_trait_double = { 
	.size  = sizeof(double),
	.equal = equal_double,
	.less  = less_double,
	.dump  = dump_double,
	.hash  = hash_double,
	.free  = do_nothing,
	.copy  = copy_double,
	.init  = init_double,
	.link  = false
};

const ax_trait ax_trait_ptr= { 
	.size  = sizeof(void *),
	.equal = equal_ptr,
	.less  = less_ptr,
	.dump  = dump_ptr,
	.hash  = hash_ptr,
	.free  = do_nothing,
	.copy  = copy_ptr,
	.init  = init_ptr,
	.link  = false
};

const ax_trait ax_trait_str = { 
	.size  = sizeof(void*),
	.equal = equal_str,
	.less  = less_str,
	.dump  = dump_str,
	.hash  = hash_str,
	.free  = free_str,
	.copy  = copy_str,
	.init  = init_str,
	.link  = true
};

const ax_trait ax_trait_wcs = { 
	.size  = sizeof(void*),
	.equal = equal_wcs,
	.less  = less_wcs,
	.dump  = dump_wcs,
	.hash  = hash_wcs,
	.free  = free_wcs,
	.copy  = copy_wcs,
	.init  = init_wcs,
	.link  = true
};

const ax_trait ax_trait_diff = { 
	.size  = sizeof(void*),
	.equal = equal_diff,
	.less  = less_diff,
	.dump  = dump_diff,
	.hash  = hash_diff,
	.free  = do_nothing,
	.copy  = copy_diff,
	.init  = init_diff,
	.link  = true
};

ax_dump *ax_trait_dump(const ax_trait *tr, const void* p)
{
	return tr->dump ? tr->dump(p) : ax_dump_symbol("?");
}

