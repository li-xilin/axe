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
#include <ax/dump.h>
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

static const struct { const char *name; int value; } st_table[] = {
	{ "",   	AX_ST_NIL },
	{ "?",  	0         },
	{ "c",  	AX_ST_C   },
	{ "f",  	AX_ST_F   },
	{ "h",  	AX_ST_H   },
	{ "i",  	AX_ST_I   },
	{ "i16",	AX_ST_I16 },
	{ "i32",	AX_ST_I32 },
	{ "i64",	AX_ST_I64 },
	{ "i8", 	AX_ST_I8  },
	{ "l",  	AX_ST_L   },
	{ "lf", 	AX_ST_LF  },
	{ "ll", 	AX_ST_LL  },
	{ "p",  	AX_ST_PTR },
	{ "s",  	AX_ST_S   },
	{ "u",  	AX_ST_U   },
	{ "u16",	AX_ST_U16 },
	{ "u32",	AX_ST_U32 },
	{ "u64",	AX_ST_U64 },
	{ "u8", 	AX_ST_U8  },
	{ "uc", 	AX_ST_UC  },
	{ "uh", 	AX_ST_UH  },
	{ "ul", 	AX_ST_UL  },
	{ "ull",	AX_ST_ULL },
	{ "ws", 	AX_ST_WS  },
	{ "z",  	AX_ST_Z   },

};

int ax_stuff_stoi(const char *s)
{
	int l = 0, r = sizeof st_table/ sizeof *st_table - 1;
	assert(r > 0);
	do {
		int m = (r - l) / 2 + l;
		int res = strcmp(st_table[m].name, s);
		if (res == 0)
			return st_table[m].value;
		if (res < 0)
			l = m + 1;
		else
			r = m - 1;
	} while (l <= r);
	return -1;
}

static ax_dump *dump_nil(const void* p, size_t size)
{
	return ax_dump_symbol("nil");
}

static ax_dump *dump_i8(const void* p, size_t size)
{
	return ax_dump_int(*(int8_t *)p);
}

static ax_dump *dump_i16(const void* p, size_t size)
{
	return ax_dump_int(*(int16_t *)p);
}

static ax_dump *dump_i32(const void* p, size_t size)
{
	return ax_dump_int(*(int32_t *)p);
}

static ax_dump *dump_i64(const void* p, size_t size)
{
	return ax_dump_int(*(int64_t *)p);
}

static ax_dump *dump_u8(const void* p, size_t size)
{
	return ax_dump_int(*(uint8_t *)p);
}

static ax_dump *dump_u16(const void* p, size_t size)
{
	return ax_dump_int(*(uint16_t *)p);
}

static ax_dump *dump_u32(const void* p, size_t size)
{
	return ax_dump_int(*(uint32_t *)p);
}

static ax_dump *dump_u64(const void* p, size_t size)
{
	return ax_dump_int(*(uint64_t *)p);
}

static ax_dump *dump_f(const void* p, size_t size)
{
	return ax_dump_float(*(float *)p);
}

static ax_dump *dump_lf(const void* p, size_t size)
{
	return ax_dump_float(*(double *)p);
}

static ax_dump *dump_z(const void* p, size_t size)
{
	return ax_dump_float(*(size_t *)p);
}

static ax_dump *dump_s(const void* p, size_t size)
{
	return ax_dump_str(*(char **)p);
}

static ax_dump *dump_ws(const void* p, size_t size)
{
	return ax_dump_wcs(*(wchar_t**)p);
}

static ax_dump *dump_ptr(const void* p, size_t size)
{
	return ax_dump_ptr(p);
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

void ax_stuff_mem_free(void* p)
{

}

ax_dump *ax_stuff_mem_dump(const void* p, size_t size)
{
	return ax_dump_mem(p, size);
}

bool ax_stuff_mem_less(const void* p1, const void* p2, size_t size)
{
	return memcmp(p1, p2, size) < 0;
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
	.dump  = dump_nil,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_i8 = { 
	.size  = sizeof(int8_t),
	.equal = equal_i8,
	.less  = less_i8,
	.dump  = dump_i8,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_i16 = { 
	.size  = sizeof(int16_t),
	.equal = equal_i16,
	.less  = less_i16,
	.dump  = dump_i16,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_i32 = { 
	.size  = sizeof(int32_t),
	.equal = equal_i32,
	.less  = less_i32,
	.dump  = dump_i32,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_i64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_i64,
	.less  = less_i64,
	.dump  = dump_i64,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u8 = { 
	.size  = sizeof(uint8_t),
	.equal = equal_u8,
	.less  = less_u8,
	.dump  = dump_u8,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u16 = { 
	.size  = sizeof(uint16_t),
	.equal = equal_u16,
	.less  = less_u16,
	.dump  = dump_u16,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u32 = { 
	.size  = sizeof(uint32_t),
	.equal = equal_u32,
	.less  = less_u32,
	.dump  = dump_u32,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_u64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_u64,
	.less  = less_u64,
	.dump  = dump_u64,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_z = { 
	.size  = sizeof(size_t),
	.equal = equal_z,
	.less  = less_z,
	.dump  = dump_z,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_f = { 
	.size  = sizeof(float),
	.equal = equal_f,
	.less  = less_f,
	.dump  = dump_f,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_lf = { 
	.size  = sizeof(double),
	.equal = equal_lf,
	.less  = less_lf,
	.dump  = dump_lf,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_ptr= { 
	.size  = sizeof(void *),
	.equal = equal_ptr,
	.less  = less_ptr,
	.dump  = dump_ptr,
	.hash  = ax_stuff_mem_hash,
	.free  = ax_stuff_mem_free,
	.copy  = ax_stuff_mem_copy,
	.init  = ax_stuff_mem_init,
	.link  = false
};

static const ax_stuff_trait trait_s = { 
	.size  = sizeof(void*),
	.equal = equal_s,
	.less  = less_s,
	.dump  = dump_s,
	.hash  = hash_s,
	.free  = free_s,
	.copy  = copy_s,
	.init  = init_s,
	.link  = true
};

static const ax_stuff_trait trait_ws = { 
	.size  = sizeof(void*),
	.equal = equal_ws,
	.less  = less_ws,
	.dump  = dump_ws,
	.hash  = hash_ws,
	.free  = free_ws,
	.copy  = copy_ws,
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

int ax_stuff_fixed_type(int type)
{
	switch(type) {
		case AX_ST_C:
			type = int_fixed_type(AX_IMAX_BITS(UCHAR_MAX));
			break;
		case AX_ST_UC:
			type = uint_fixed_type(AX_IMAX_BITS(UCHAR_MAX));
			break;
		case AX_ST_H:
			type = int_fixed_type(AX_IMAX_BITS(USHRT_MAX));
			break;
		case AX_ST_UH:
			type = uint_fixed_type(AX_IMAX_BITS(USHRT_MAX));
			break;
		case AX_ST_I:
			type = int_fixed_type(AX_IMAX_BITS(UINT_MAX));
			break;
		case AX_ST_U:
			type = uint_fixed_type(AX_IMAX_BITS(UINT_MAX));
			break;
		case AX_ST_L:
			type = int_fixed_type(AX_IMAX_BITS(ULONG_MAX));
			break;
		case AX_ST_UL:
			type = uint_fixed_type(AX_IMAX_BITS(ULONG_MAX));
			break;
		case AX_ST_LL:
			type = uint_fixed_type(AX_IMAX_BITS(ULLONG_MAX));
			break;
		case AX_ST_ULL:
			type = uint_fixed_type(AX_IMAX_BITS(ULLONG_MAX));
			break;
	}
	return type;
}

/* -- Define functions that get trait structure pointer -- */
const ax_stuff_trait* ax_stuff_traits(int type)
{
	CHECK_PARAM_VALIDITY(type, type >= 0);
	type = ax_stuff_fixed_type(type);

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

ax_dump *ax_stuff_dump(const ax_stuff_trait *tr, const void* p, size_t size)
{
	return tr->dump ? tr->dump(p, size) : ax_dump_symbol("?");
}

