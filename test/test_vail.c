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

#include "ax/vail.h"
#include "ax/base.h"
#include "ax/stuff.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void test_vail(axut_runner *r)
{
	uint8_t array[5] = { 42, 42, 42, 42, 42 };
	ax_vail *vail = ax_vail_create("i8_i16_i32_i64_u8_u16_u32_u64_z_f_lf_s_&i8",
			(int8_t)-8,
			(int16_t)-16,
			(int32_t)-32,
			(int64_t)-64,
			(uint8_t)8,
			(uint16_t)16,
			(uint32_t)32,
			(uint64_t)64,
			(size_t)42,
			(float)42,
			(double)42,
			"helloworld",
			array, (size_t)5);
	for (int i = 0; i < ax_vail_size(vail); i++) {
		ax_vail_info info;
		ax_vail_get(vail, i, &info);
		switch(i) {
			case 0:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_I8);
				axut_assert(r, *(int8_t*)info.ptr == -8);
				break;
			case 1:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_I16);
				axut_assert(r, *(int16_t*)info.ptr == -16);
				break;
			case 2:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_I32);
				axut_assert(r, *(int32_t*)info.ptr == -32);
				break;
			case 3:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_I64);
				axut_assert(r, *(int64_t*)info.ptr == -64);
				break;
			case 4:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_U8);
				axut_assert(r, *(uint8_t*)info.ptr == 8);
				break;
			case 5:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_U16);
				axut_assert(r, *(uint16_t*)info.ptr == 16);
				break;
			case 6:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_U32);
				axut_assert(r, *(uint32_t*)info.ptr == 32);
				break;
			case 7:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_U64);
				axut_assert(r, *(uint64_t*)info.ptr == 64);
				break;
			case 8:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_Z);
				axut_assert(r, *(size_t*)info.ptr == 42);
				break;
			case 9:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_F);
				axut_assert(r, (int)*(float*)info.ptr == 42);
				break;
			case 10:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_LF);
				axut_assert(r, (int)*(double*)info.ptr == 42);
				break;
			case 11:
				axut_assert(r, info.size == 1);
				axut_assert(r, info.type == AX_ST_S);
				axut_assert_str_equal(r, *(char**)info.ptr, "helloworld");
				break;
			case 12:
				axut_assert(r, info.size == 5);
				axut_assert(r, info.type == AX_ST_I8);
				axut_assert(r, ((uint8_t*)info.ptr)[0] == 42);
				break;
		}
	}
	ax_vail_destroy(vail);

	vail = ax_vail_create("");
	axut_assert(r, ax_vail_size(vail) == 0);

	ax_vail_destroy(vail);

}
axut_suite *suite_for_vail(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "vail");

	axut_suite_add(suite, test_vail, 0);

	return suite;
}
