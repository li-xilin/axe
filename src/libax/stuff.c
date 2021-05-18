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

#include <ax/stuff.h>
#include <ax/debug.h>
#include <ax/mem.h>
#include <ax/def.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <inttypes.h>
#include <assert.h>

#include "check.h"


inline static int int_fixed_type(int bits);

size_t ax_stuff_size(int type)
{
	switch(type) {
		case AX_ST_NIL:  return 0;
		case AX_ST_I8:   return sizeof(int8_t);
		case AX_ST_I16:  return sizeof(int16_t);
		case AX_ST_I32:  return sizeof(int32_t);
		case AX_ST_I64:  return sizeof(int64_t);
		case AX_ST_U8:   return sizeof(uint8_t);
		case AX_ST_U16:  return sizeof(uint16_t);
		case AX_ST_U32:  return sizeof(uint32_t);
		case AX_ST_U64:  return sizeof(uint64_t);
		case AX_ST_Z:    return sizeof(size_t);
		case AX_ST_F:    return sizeof(float);
		case AX_ST_LF:   return sizeof(double);
		case AX_ST_S:
		case AX_ST_WS:
		case AX_ST_PTR:  return sizeof(void*);
	}
	ax_assert(false, "unrecognized type %d", type);
	return 0;
}

static bool equal_nil(const void* p1, const void* p2, size_t size)
{
	return true;
}

static bool equal_i8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 == *(int8_t*) p2;
}

static bool equal_i16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 == *(int16_t*)p2;
}

static bool equal_i32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 == *(int32_t*)p2;
}

static bool equal_i64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 == *(int64_t*)p2;
}

static bool equal_u8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 == *(int8_t*) p2;
}

static bool equal_u16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 == *(int16_t*)p2;
}

static bool equal_u32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 == *(int32_t*)p2;
}

static bool equal_u64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 == *(int64_t*)p2;
}

static bool equal_f(const void* p1, const void* p2, size_t size)
{
	return *(float*)  p1 == *(float*)  p2;
}

static bool equal_lf(const void* p1, const void* p2, size_t size)
{
	return *(double*) p1 == *(double*) p2;
}

static bool equal_z(const void* p1, const void* p2, size_t size)
{
	return *(size_t*) p1 == *(size_t*) p2;
}

static bool equal_s(const void* p1, const void* p2, size_t size)
{
	return strcmp(*(char**)p1, *(char**)p2) == 0;
}

static bool equal_ws(const void* p1, const void* p2, size_t size)
{
	return wcscmp(*(wchar_t**)p1, *(wchar_t**)p2) == 0;
}

static bool equal_ptr(const void* p1, const void* p2, size_t size)
{
	return *(void**) p1 == *(void**) p2;
}

static bool less_nil(const void* p1, const void* p2, size_t size)
{
	return false;
}

static bool less_i8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 < *(int8_t*) p2;
}

static bool less_i16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 < *(int16_t*)p2;
}

static bool less_i32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 < *(int32_t*)p2;
}

static bool less_i64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 < *(int64_t*)p2;
}

static bool less_u8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 < *(int8_t*) p2;
}

static bool less_u16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 < *(int16_t*)p2;
}

static bool less_u32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 < *(int32_t*)p2;
}

static bool less_u64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 < *(int64_t*)p2;
}

static bool less_f(const void* p1, const void* p2, size_t size)
{
	return *(float*)  p1 < *(float*)  p2;
}

static bool less_lf(const void* p1, const void* p2, size_t size)
{
	return *(double*) p1 < *(double*) p2;
}

static bool less_z(const void* p1, const void* p2, size_t size)
{
	return *(size_t*) p1 < *(size_t*) p2;
}

static bool less_s(const void* p1, const void* p2, size_t size)
{
	return strcmp(*(char**)p1, *(char**)p2) < 0;
}

static bool less_ws(const void* p1, const void* p2, size_t size)
{
	return wcscmp(*(wchar_t**)p1, *(wchar_t**)p2) < 0;
}

static bool less_ptr(const void* p1, const void* p2, size_t size)
{
	return *(intptr_t**) p1 < *(intptr_t**) p2;
}


static ax_fail copy_s(void* dst, const void* src, size_t size)
{
	return !((*(char**)dst = ax_strdup(*(char**)src)));
	
}

static ax_fail copy_ws(void* dst, const void* src, size_t size)
{
	return !(*(wchar_t**)dst = ax_wcsdup(*(wchar_t**)src));
}

static void move_s(void* dst, const void* src, size_t size)
{
	*(char**)dst = *(char**)src;
	*(char**)src = NULL;
}

static void move_ws(void* dst, const void* src, size_t size)
{
	*(wchar_t**)dst = *(wchar_t**)src;
	*(wchar_t**)src = NULL;
}


static ax_fail init_s(void* p, size_t size) {
	char *s = malloc(sizeof(char));
	if (s == NULL)
		return true;
	*s = '\0';
	*(void**)  p = s;
	return false;
}

static bool init_ws(void* p, size_t size) {
	wchar_t *s = malloc(sizeof(wchar_t));
	if (s == NULL)
		return true;
	*s = L'\0';
	*(void**)  p = s;
	return false;
}


static size_t hash_s(const void* p, size_t size)
{
	return ax_strhash(*(char**)p);
} 

static size_t hash_ws(const void* p, size_t size)
{
	return ax_wcshash(*(wchar_t**)p);
} 

void ax_stuff_mem_free(void* p) {

}

bool ax_stuff_mem_less(const void* p1, const void* p2, size_t size)
{
	return memcmp(p1, p2, size);
}

bool ax_stuff_mem_equal(const void* p1, const void* p2, size_t size)
{
	return memcmp(p1, p2, size) == 0;
}

size_t ax_stuff_mem_hash(const void* p, size_t size)
{
	return ax_memhash(p, size);
}

ax_fail ax_stuff_mem_copy(void* dst, const void* src, size_t size)
{
	switch(size) {
		case 1: *(uint8_t *)dst = *(uint8_t *)src; break;
		case 2: *(uint16_t *)dst = *(uint16_t *)src; break;
		case 4: *(uint32_t *)dst = *(uint32_t *)src; break;
		case 8: *(uint64_t *)dst = *(uint64_t *)src; break;
		default: memcpy(dst, src, size); break;
	}
	return false;
}

void ax_stuff_mem_move(void* dst, const void* src, size_t size)
{
	ax_stuff_mem_copy(dst, src, size);
}

void ax_stuff_mem_swap(void* dst, void* src, size_t size)
{
	switch(size) {
		case 0: return;
		case sizeof(uint8_t):
			ax_mem_pswap(dst, src, uint8_t);
			break;
		case sizeof(uint16_t):
			ax_mem_pswap(dst, src, uint16_t);
			break;
		case sizeof(uint32_t):
			ax_mem_pswap(dst, src, uint32_t);
			break;
		case sizeof(uint64_t):
			ax_mem_pswap(dst, src, uint64_t);
			break;
		default:
			ax_memxor(dst, src, size);
			ax_memxor(src, dst, size);
			ax_memxor(dst, src, size);
	}
}


ax_fail ax_stuff_mem_init(void* p, size_t size)
{
	memset(p, 0, size);
	return false;
}

static void free_s(void* p)
{
	free(*(char**)p);
}

static void free_ws(void* p)
{
	free(*(char**)p);
}

static const ax_stuff_trait trait_nil = { 
	.size  = 0,
	.equal = equal_nil,
	.less  = less_nil,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};


static const ax_stuff_trait trait_i8 = { 
	.size  = sizeof(int8_t),
	.equal = equal_i8,
	.less  = less_i8,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_i16 = { 
	.size  = sizeof(int16_t),
	.equal = equal_i16,
	.less  = less_i16,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_i32 = { 
	.size  = sizeof(int32_t),
	.equal = equal_i32,
	.less  = less_i32,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_i64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_i64,
	.less  = less_i64,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u8 = { 
	.size  = sizeof(uint8_t),
	.equal = equal_u8,
	.less  = less_u8,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u16 = { 
	.size  = sizeof(uint16_t),
	.equal = equal_u16,
	.less  = less_u16,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u32 = { 
	.size  = sizeof(uint32_t),
	.equal = equal_u32,
	.less  = less_u32,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_u64,
	.less  = less_u64,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_z = { 
	.size  = sizeof(size_t),
	.equal = equal_z,
	.less  = less_z,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_f = { 
	.size  = sizeof(float),
	.equal = equal_f,
	.less  = less_f,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_lf = { 
	.size  = sizeof(double),
	.equal = equal_lf,
	.less  = less_lf,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_ptr= { 
	.size  = sizeof(void *),
	.equal = equal_ptr,
	.less  = less_ptr,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.move  = ax_stuff_mem_move,
	.swap  = ax_stuff_mem_swap,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_s = { 
	.size  = sizeof(void*),
	.equal = equal_s,
	.less  = less_s,
	.hash  = hash_s,
	.free  = free_s,
	.copy  = copy_s,
	.move  = move_s,
	.swap  = ax_stuff_mem_swap,
	.init  = init_s,
	.link  = true
};

static const ax_stuff_trait trait_ws = { 
	.size  = sizeof(void*),
	.equal = equal_ws,
	.less  = less_ws,
	.hash  = hash_ws,
	.free  = free_ws,
	.copy  = copy_ws,
	.move  = move_ws,
	.swap  = ax_stuff_mem_swap,
	.init  = init_ws,
	.link  = true
};

inline static int int_fixed_type(int bits)
{
	switch(bits) {
		case 0: return AX_ST_NIL;
		case 8: return AX_ST_I8;
		case 16: return AX_ST_I16;
		case 32: return AX_ST_I32;
		case 64: return AX_ST_I64;
		default: ax_assert(0, "invalid bits value");
			 return -1;
	}
}

inline static int uint_fixed_type(int bits)
{
	switch(bits) {
		case 0: return AX_ST_NIL;
		case 8: return AX_ST_U8;
		case 16: return AX_ST_U16;
		case 32: return AX_ST_U32;
		case 64: return AX_ST_U64;
		default: ax_assert(0, "invalid bits value");
			 return -1;
	}
}

/* -- Define functions that get trait structure pointer -- */
const ax_stuff_trait* ax_stuff_traits(int type)
{
	CHECK_PARAM_VALIDITY(type, type >= 0);
	switch(type) {
		case AX_ST_C:
			type = int_fixed_type(AX_IMAX_BITS((unsigned char)-1));
			break;
		case AX_ST_H:
			type = int_fixed_type(AX_IMAX_BITS((unsigned short int)-1));
			break;
		case AX_ST_I:
			type = int_fixed_type(AX_IMAX_BITS((unsigned int)-1));
			break;
		case AX_ST_L:
			type = int_fixed_type(AX_IMAX_BITS((unsigned long int)-1));
			break;
		case AX_ST_LL:
			type = int_fixed_type(AX_IMAX_BITS((unsigned long long int)-1));
			break;
		case AX_ST_UC:
			type = uint_fixed_type(AX_IMAX_BITS((unsigned char)-1));
			break;
		case AX_ST_UH:
			type = uint_fixed_type(AX_IMAX_BITS((unsigned short int)-1));
			break;
		case AX_ST_U:
			type = uint_fixed_type(AX_IMAX_BITS((unsigned int)-1));
			break;
		case AX_ST_UL:
			type = uint_fixed_type(AX_IMAX_BITS((unsigned long int)-1));
			break;
		case AX_ST_ULL:
			type = uint_fixed_type(AX_IMAX_BITS((unsigned long long int)-1));
			break;
	}

	switch(type) {
		case AX_ST_NIL:  return &trait_nil;
		case AX_ST_I8:   return &trait_i8;
		case AX_ST_I16:  return &trait_i16;
		case AX_ST_I32:  return &trait_i32;
		case AX_ST_I64:  return &trait_i64;
		case AX_ST_U8:   return &trait_u8;
		case AX_ST_U16:  return &trait_u16;
		case AX_ST_U32:  return &trait_u32;
		case AX_ST_U64:  return &trait_u64;
		case AX_ST_Z:    return &trait_z;
		case AX_ST_F:    return &trait_f;
		case AX_ST_LF:   return &trait_lf;
		case AX_ST_S:    return &trait_s;
		case AX_ST_WS:   return &trait_ws;
		case AX_ST_PTR:  return &trait_ptr;
		default:         return NULL;
	}
}
