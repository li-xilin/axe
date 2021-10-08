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

#include <stdint.h>
#include "debug.h"

#define __AX_ARRAYA_MAGIC "@arraya"

#define __ax_sizeof_arr(_t, ...) \
	sizeof((_t[]){ __VA_ARGS__ })

#define ax_arraya(_t, ...) \
	(_t *) ( \
	  ( \
	    ( \
	      struct { \
	        uint64_t _len; \
	       	char _mag[8]; \
	        _t _arr[__ax_sizeof_arr(_t, __VA_ARGS__) / sizeof(_t)]; \
	      } [1] \
	    ) \
	    { \
	      { \
	        ._len = __ax_sizeof_arr(_t, __VA_ARGS__), \
	        ._mag = __AX_ARRAYA_MAGIC, \
		._arr = { __VA_ARGS__}, \
	      } \
	    } \
	  ) ->_arr \
	)

inline static size_t ax_arraya_size(const void *arra)
{
	struct arraya_hdr {
	        uint64_t _len;
		char _mag[8];
		char data[];
	} *blk = (void *)(((char *)arra) - sizeof(struct arraya_hdr));

	ax_assert(*(uint64_t*)blk->_mag == *(uint64_t *)__AX_ARRAYA_MAGIC, "invalid arraya pointer");
	return (size_t)blk->_len;
}

#endif