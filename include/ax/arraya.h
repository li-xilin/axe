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

#ifndef AX_ARRAYA_H
#define AX_ARRAYA_H

#include "debug.h"
#include "narg.h"
#include <stdint.h>
#include <stddef.h>

#define __AX_ARRAYA_MAGIC "@arraya"

#define ax_arraya(_t, ...) \
	(_t *) ( \
	  ( \
	    ( \
	      struct { \
	        uint64_t _size; \
	       	char _magic[8]; \
	        _t _arr[AX_NARG_T(_t, __VA_ARGS__)]; \
	      } [1] \
	    ) \
	    { \
	      { \
	        ._size = AX_NARG_T(_t, __VA_ARGS__) * sizeof(_t), \
	        ._magic = __AX_ARRAYA_MAGIC, \
		._arr = { __VA_ARGS__}, \
	      } \
	    } \
	  ) ->_arr \
	)

inline static size_t ax_arraya_size(const void *arra)
{
	struct arraya_hdr {
	        uint64_t _size;
		char _magic[8];
		char data[];
	} *blk = (struct arraya_hdr *)(((char *)arra) - sizeof(struct arraya_hdr));

	ax_assert(*(uint64_t*)blk->_magic == *(uint64_t *)__AX_ARRAYA_MAGIC, "invalid arraya pointer");
	return (size_t)blk->_size;
}

#endif
