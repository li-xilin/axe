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

#ifndef AXE_STUFF_H_
#define AXE_STUFF_H_
#include "def.h"
#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <limits.h>

#ifndef AX_STUFF_TRAIT_DEFINED
#define AX_STUFF_TRAIT_DEFINED
typedef struct ax_stuff_trait_st ax_stuff_trait;
#endif

#ifndef AX_DUMP_DEFINED
#define AX_DUMP_DEFINED
typedef struct ax_dump_st ax_dump;
#endif



#define AX_ST_NIL   1
#define AX_ST_I8    2
#define AX_ST_I16   3
#define AX_ST_I32   4
#define AX_ST_I64   5
#define AX_ST_U8    6
#define AX_ST_U16   7
#define AX_ST_U32   8
#define AX_ST_U64   9
#define AX_ST_Z     10
#define AX_ST_F     11
#define AX_ST_LF    12
#define AX_ST_S     13
#define AX_ST_WS    14
#define AX_ST_PTR   15

#define AX_ST_C     16
#define AX_ST_H     17
#define AX_ST_I     18
#define AX_ST_L     19
#define AX_ST_LL    20

#define AX_ST_UC    21
#define AX_ST_UH    22
#define AX_ST_U     23
#define AX_ST_UL    24
#define AX_ST_ULL   25

#define AX_ST_PWL   26

union ax_stuff_un
{
	int8_t   i8;
	int16_t  i16;
	int32_t  i32;
	int64_t  i64;
	uint8_t  u8;
	uint16_t u16;
	uint32_t u32;
	uint64_t u64;
	size_t   z;
	float    f;
	double   lf;
	char*    str;
	wchar_t* wstr;
	void*    ptr;
	void*    raw;
};
typedef union ax_stuff_un ax_stuff;

typedef void    (*ax_stuff_free_f) (void* p);
typedef bool    (*ax_stuff_compare_f) (const void* p1, const void* p2, size_t size);
typedef size_t  (*ax_stuff_hash_f) (const void* p, size_t size);
typedef void    (*ax_stuff_move_f) (void* dst, const void* src, size_t size);
typedef void    (*ax_stuff_swap_f) (void* dst, void* src, size_t size);
typedef ax_fail (*ax_stuff_copy_f) (void* dst, const void* src, size_t size);
typedef ax_fail (*ax_stuff_init_f) (void* p, size_t size);
typedef ax_dump*(*ax_stuff_dump_f) (const void* p, size_t size);

struct ax_stuff_trait_st
{
	const size_t       size;
	ax_stuff_compare_f equal;
	ax_stuff_compare_f less;
	ax_stuff_hash_f    hash;
	ax_stuff_free_f    free; 
	ax_stuff_copy_f    copy;
	ax_stuff_move_f    move;
	ax_stuff_swap_f    swap;
	ax_stuff_init_f    init;
	ax_stuff_dump_f    dump;
	bool               link;
};

bool ax_stuff_mem_equal(const void* p1, const void* p2, size_t size);
bool ax_stuff_mem_less(const void* p1, const void* p2, size_t size);
size_t  ax_stuff_mem_hash(const void* p, size_t size);
void    ax_stuff_mem_move(void* dst, const void* src, size_t size);
ax_fail ax_stuff_mem_copy(void* dst, const void* src, size_t size);
void    ax_stuff_mem_swap(void* dst, void* src, size_t size);
ax_fail ax_stuff_mem_init(void* p, size_t size);
void    ax_stuff_mem_free(void* p);
ax_dump *ax_stuff_mem_dump(const void* p, size_t size);


size_t ax_stuff_size(int type);

const ax_stuff_trait *ax_stuff_traits(int type);

int ax_stuff_stoi(const char *s);

#endif
