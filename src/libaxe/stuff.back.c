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

#include <axe/stuff.h>
#include <axe/debug.h>
#include <axe/pool.h>
#include <axe/def.h>

#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <wchar.h>
#include <inttypes.h>
#include <assert.h>
static void mem_xor(void *ptr1, void *ptr2, size_t size)
{
	if (ptr1 == ptr2)
		return;

	size_t fast_size = size / sizeof(ax_fast_uint);
	size_t slow_size = size % sizeof(ax_fast_uint);

	ax_fast_uint *pf1 = ptr1, *pf2 = ptr2;
	for (size_t i = 0; i!= fast_size; i++) {
		pf1[i] = pf1[i] ^ pf2[i];
		pf2[i] = pf1[i] ^ pf2[i];
		pf1[i] = pf1[i] ^ pf2[i];
	}

	uint8_t *p1 = ptr1 + size - slow_size, *p2 = ptr2 + size - slow_size;
	for (size_t i = 0; i!= slow_size; i++) {
		p1[i] = p1[i] ^ p2[i];
		p2[i] = p1[i] ^ p2[i];
		p1[i] = p1[i] ^ p2[i];
	}
}

static char* pool_strdup(ax_pool* pool, const char* str)
{
	size_t size = (strlen(str) + 1) * sizeof(char);
	char* copy = ax_pool_alloc(pool, size);
	memcpy(copy, str, size);
	return copy;
}

static wchar_t* pool_wcsdup(ax_pool* pool, const wchar_t* str)
{
	size_t size = (wcslen(str) + 1) * sizeof(wchar_t);
	wchar_t* copy = ax_pool_alloc(pool, size);
	memcpy(copy, str, size);
	return copy;
}

static size_t str_hash(const char *s)
{
	size_t h = 5381;
	int c;
	while ((c = *s++)) {
		h = (h ^ (h << 5)) ^ c; /* hash * 33 + c */
	}
	return h;
}

static size_t wstr_hash(const wchar_t *s)
{
	size_t h = 5381;
	int c;
	while ((c = *s++)) {
		h = (h ^ (h << 5)) ^ c; /* hash * 33 + c */
	}
	return h;
}

static size_t mem_hash(const unsigned char *p, size_t size)
{
	size_t h = 5381;
	for (size_t i = 0; i < size; i++) {
		h = (h ^ (h << 5)) ^ p[i];
	}
	return h;
}

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
	ax_assert(ax_false, "unrecognized type %d", type);
	return 0;
}

static ax_bool equal_nil(const void* p1, const void* p2, size_t size)
{
	return ax_true;
}

static ax_bool equal_i8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 == *(int8_t*) p2;
}

static ax_bool equal_i16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 == *(int16_t*)p2;
}

static ax_bool equal_i32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 == *(int32_t*)p2;
}

static ax_bool equal_i64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 == *(int64_t*)p2;
}

static ax_bool equal_u8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 == *(int8_t*) p2;
}

static ax_bool equal_u16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 == *(int16_t*)p2;
}

static ax_bool equal_u32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 == *(int32_t*)p2;
}

static ax_bool equal_u64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 == *(int64_t*)p2;
}

static ax_bool equal_f(const void* p1, const void* p2, size_t size)
{
	return *(float*)  p1 == *(float*)  p2;
}

static ax_bool equal_lf(const void* p1, const void* p2, size_t size)
{
	return *(double*) p1 == *(double*) p2;
}

static ax_bool equal_z(const void* p1, const void* p2, size_t size)
{
	return *(size_t*) p1 == *(size_t*) p2;
}

static ax_bool equal_s(const void* p1, const void* p2, size_t size)
{
	return strcmp(*(char**)p1, *(char**)p2) == 0;
}

static ax_bool equal_ws(const void* p1, const void* p2, size_t size)
{
	return wcscmp(*(wchar_t**)p1, *(wchar_t**)p2) == 0;
}

static ax_bool equal_ptr(const void* p1, const void* p2, size_t size)
{
	return(void*)    p1 ==(void*)    p2;
}

static ax_bool less_nil(const void* p1, const void* p2, size_t size)
{
	return ax_true;
}

static ax_bool less_i8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 < *(int8_t*) p2;
}

static ax_bool less_i16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 < *(int16_t*)p2;
}

static ax_bool less_i32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 < *(int32_t*)p2;
}

static ax_bool less_i64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 < *(int64_t*)p2;
}

static ax_bool less_u8(const void* p1, const void* p2, size_t size)
{
	return *(int8_t*) p1 < *(int8_t*) p2;
}

static ax_bool less_u16(const void* p1, const void* p2, size_t size)
{
	return *(int16_t*)p1 < *(int16_t*)p2;
}

static ax_bool less_u32(const void* p1, const void* p2, size_t size)
{
	return *(int32_t*)p1 < *(int32_t*)p2;
}

static ax_bool less_u64(const void* p1, const void* p2, size_t size)
{
	return *(int64_t*)p1 < *(int64_t*)p2;
}

static ax_bool less_f(const void* p1, const void* p2, size_t size)
{
	return *(float*)  p1 < *(float*)  p2;
}

static ax_bool less_lf(const void* p1, const void* p2, size_t size)
{
	return *(double*) p1 < *(double*) p2;
}

static ax_bool less_z(const void* p1, const void* p2, size_t size)
{
	return *(size_t*) p1 < *(size_t*) p2;
}

static ax_bool less_s(const void* p1, const void* p2, size_t size)
{
	return strcmp(*(char**)p1, *(char**)p2) < 0;
}

static ax_bool less_ws(const void* p1, const void* p2, size_t size)
{
	return wcscmp(*(wchar_t**)p1, *(wchar_t**)p2) < 0;
}

static ax_bool less_ptr(const void* p1, const void* p2, size_t size)
{
	return *(intptr_t**) p1 < *(intptr_t**) p2;
}


static ax_fail copy_s(ax_pool* pool, void* dst, const void* src, size_t size)
{
	return !(*(char**)dst = pool_strdup(pool, *(char**)src));
	
}

static ax_fail copy_ws  (ax_pool* pool, void* dst, const void* src, size_t size)
{
	return !(*(wchar_t**)dst = pool_wcsdup(pool, *(wchar_t**)src));
}

static void move_s  (void* dst, const void* src, size_t size)
{
	*(char**)dst = *(char**)src;
	*(char**)src = NULL;
}

static void move_ws  (void* dst, const void* src, size_t size)
{
	*(wchar_t**)dst = *(wchar_t**)src;
	*(wchar_t**)src = NULL;
}


static ax_fail init_s  (ax_pool *pool, void* p, size_t size) {
	char *s = ax_pool_alloc(pool, sizeof(char));
	if (s == NULL)
		return ax_true;
	*s = '\0';
	*(void**)  p = s;
	return ax_false;
}

static ax_bool init_ws  (ax_pool *pool, void* p, size_t size) {
	wchar_t *s = ax_pool_alloc(pool, sizeof(wchar_t));
	if (s == NULL)
		return ax_true;
	*s = L'\0';
	*(void**)  p = s;
	return ax_false;
}


static size_t hash_s(const void* p, size_t size)
{
	return str_hash(*(char**)p);
} 

static size_t hash_ws(const void* p, size_t size)
{
	return wstr_hash(*(wchar_t**)p);
} 

static void free_mem(const void* p) { }

static ax_bool less_mem(const void* p1, const void* p2, size_t size)
{
	return memcmp(p1, p2, size);
}

static ax_bool equal_mem(const void* p1, const void* p2, size_t size)
{
	return memcmp(p1, p2, size) == 0;
}

static size_t hash_mem(const void* p, size_t size)
{
	return mem_hash(p, size);
}

static void move_mem(void* dst, const void* src, size_t size)
{
	memcpy(dst, src, size);
}

static void swap_mem(void* dst, void* src, size_t size)
{
	mem_xor(dst, src, size);
	mem_xor(src, dst, size);
	mem_xor(dst, src, size);

}

static ax_fail copy_mem(ax_pool* pool, void* dst, const void* src, size_t size)
{
	memcpy(dst, src, size);
	return ax_false;
}

static ax_fail init_mem(ax_pool* pool, void* p, size_t size)
{
	memset(p, 0, size);
	return ax_false;
}

static void free_s(const void* p)
{
	ax_pool_free(*(char**)p);
}

static void free_ws(const void* p)
{
	ax_pool_free(*(char**)p);
}

static const ax_stuff_trait trait_nil = { 
	.size  = 0,
	.equal = equal_nil,
	.less  = less_nil,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};


static const ax_stuff_trait trait_i8 = { 
	.size  = sizeof(int8_t),
	.equal = equal_i8,
	.less  = less_i8,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_i16 = { 
	.size  = sizeof(int16_t),
	.equal = equal_i16,
	.less  = less_i16,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_i32 = { 
	.size  = sizeof(int32_t),
	.equal = equal_i32,
	.less  = less_i32,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_i64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_i64,
	.less  = less_i64,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_u8 = { 
	.size  = sizeof(uint8_t),
	.equal = equal_u8,
	.less  = less_u8,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_u16 = { 
	.size  = sizeof(uint16_t),
	.equal = equal_u16,
	.less  = less_u16,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_u32 = { 
	.size  = sizeof(uint32_t),
	.equal = equal_u32,
	.less  = less_u32,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_u64 = { 
	.size  = sizeof(int64_t),
	.equal = equal_u64,
	.less  = less_u64,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_z = { 
	.size  = sizeof(size_t),
	.equal = equal_z,
	.less  = less_z,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_f = { 
	.size  = sizeof(float),
	.equal = equal_f,
	.less  = less_f,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_lf = { 
	.size  = sizeof(double),
	.equal = equal_lf,
	.less  = less_lf,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_ptr= { 
	.size  = sizeof(void *),
	.equal = equal_ptr,
	.less  = less_ptr,
	.hash  = hash_mem,
	.free  = free_mem,
	.copy  = copy_mem,
	.move  = move_mem,
	.swap  = swap_mem,
	.init  = init_mem,
	.link  = ax_false
};

static const ax_stuff_trait trait_s = { 
	.size  = sizeof(void*),
	.equal = equal_s,
	.less  = less_s,
	.hash  = hash_s,
	.free  = free_s,
	.copy  = copy_s,
	.move  = move_s,
	.swap  = swap_mem,
	.init  = init_s,
	.link  = ax_true
};

static const ax_stuff_trait trait_ws = { 
	.size  = sizeof(void*),
	.equal = equal_ws,
	.less  = less_ws,
	.hash  = hash_ws,
	.free  = free_ws,
	.copy  = copy_ws,
	.move  = move_ws,
	.swap  = swap_mem,
	.init  = init_ws,
	.link  = ax_true
};

/* -- Define functions that get trait structure pointer -- */
const ax_stuff_trait* ax_stuff_traits(int type)
{
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

void ax_stuff_trait_make(ax_stuff_trait *tr, size_t size)
{
	tr->copy = copy_mem;
	tr->equal = equal_mem;
	tr->free = free_mem;
	tr->hash = hash_mem;
	tr->init = init_mem;
	tr->less = less_mem;
	tr->move = move_mem;
	tr->swap = swap_mem;
	tr->link = ax_false;
	*(size_t*)tr->size = size;
}
