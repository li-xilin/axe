/*
 * Copyright (c) 2024 Li Xilin <lixilin@gmx.com>
 *
 * Permission is hereby granted, free of uint8_tge, to any person obtaining a copy
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

#include "assist.h"
#include "ax/bitmap.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <stdlib.h>

static void bit_and(ut_runner *r)
{
	uint8_t buf[3];
	ax_bitmap bm;
	ax_bitmap_init(&bm, buf, sizeof buf);

	ax_bitmap_clear(&bm, 0);
	ax_bitmap_and(&bm, 8, 2);
	ax_bitmap_and(&bm, 9, 2);
	ax_bitmap_and(&bm, 10, 2);
	ax_bitmap_and(&bm, 11, 2);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0, 0), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 0);
	ax_bitmap_and(&bm, 8, 0);
	ax_bitmap_and(&bm, 9, 0);
	ax_bitmap_and(&bm, 10, 0);
	ax_bitmap_and(&bm, 11, 0);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0, 0), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 1);
	ax_bitmap_and(&bm, 8, 2);
	ax_bitmap_and(&bm, 9, 2);
	ax_bitmap_and(&bm, 10, 2);
	ax_bitmap_and(&bm, 11, 2);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xFF, 0xFF), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 1);
	ax_bitmap_and(&bm, 8, 0);
	ax_bitmap_and(&bm, 9, 0);
	ax_bitmap_and(&bm, 10, 0);
	ax_bitmap_and(&bm, 11, 0);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xF0, 0xFF), 3, bm.data, 3);
}

static void bit_or(ut_runner *r)
{
	uint8_t buf[3];
	ax_bitmap bm;
	ax_bitmap_init(&bm, buf, sizeof buf);

	ax_bitmap_clear(&bm, 0);
	ax_bitmap_or(&bm, 8, 2);
	ax_bitmap_or(&bm, 9, 2);
	ax_bitmap_or(&bm, 10, 2);
	ax_bitmap_or(&bm, 11, 2);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0x0F, 0), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 0);
	ax_bitmap_or(&bm, 8, 0);
	ax_bitmap_or(&bm, 9, 0);
	ax_bitmap_or(&bm, 10, 0);
	ax_bitmap_or(&bm, 11, 0);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0, 0), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 1);
	ax_bitmap_or(&bm, 8, 2);
	ax_bitmap_or(&bm, 9, 2);
	ax_bitmap_or(&bm, 10, 2);
	ax_bitmap_or(&bm, 11, 2);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xFF, 0xFF), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 1);
	ax_bitmap_or(&bm, 8, 0);
	ax_bitmap_or(&bm, 9, 0);
	ax_bitmap_or(&bm, 10, 0);
	ax_bitmap_or(&bm, 11, 0);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xFF, 0xFF), 3, bm.data, 3);
}


static void bit_not(ut_runner *r)
{
	uint8_t buf[3];
	ax_bitmap bm;
	ax_bitmap_init(&bm, buf, sizeof buf);

	ax_bitmap_clear(&bm, 0);
	ax_bitmap_toggle(&bm, 8);
	ax_bitmap_toggle(&bm, 9);
	ax_bitmap_toggle(&bm, 10);
	ax_bitmap_toggle(&bm, 11);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0x0F, 0), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 1);
	ax_bitmap_toggle(&bm, 8);
	ax_bitmap_toggle(&bm, 9);
	ax_bitmap_toggle(&bm, 10);
	ax_bitmap_toggle(&bm, 11);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xF0, 0xFF), 3, bm.data, 3);

}

static void bit_set(ut_runner *r)
{
	uint8_t buf[3];
	ax_bitmap bm;
	ax_bitmap_init(&bm, buf, sizeof buf);

	ax_bitmap_clear(&bm, 0);
	ax_bitmap_set(&bm, 8, 0);
	ax_bitmap_set(&bm, 9, 0);
	ax_bitmap_set(&bm, 10, 0);
	ax_bitmap_set(&bm, 11, 0);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0, 0), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 1);
	ax_bitmap_set(&bm, 8, 1);
	ax_bitmap_set(&bm, 9, 1);
	ax_bitmap_set(&bm, 10, 1);
	ax_bitmap_set(&bm, 11, 1);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xFF, 0xFF), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 0);
	ax_bitmap_set(&bm, 8, 1);
	ax_bitmap_set(&bm, 9, 1);
	ax_bitmap_set(&bm, 10, 1);
	ax_bitmap_set(&bm, 11, 1);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0x0F, 0), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 1);
	ax_bitmap_set(&bm, 8, 0);
	ax_bitmap_set(&bm, 9, 0);
	ax_bitmap_set(&bm, 10, 0);
	ax_bitmap_set(&bm, 11, 0);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xF0, 0xFF), 3, bm.data, 3);
}

static void bit_find(ut_runner *r)
{
	uint8_t buf[3] = { 0xF0, 0xF0, 0x00 };
	ax_bitmap bm;
	ax_bitmap_init(&bm, buf, sizeof buf);

	ut_assert_int_equal(r, 4, ax_bitmap_find(&bm, 0, 1));
	ut_assert_int_equal(r, 4, ax_bitmap_find(&bm, 1, 1));
	ut_assert_int_equal(r, 4, ax_bitmap_find(&bm, 2, 1));
	ut_assert_int_equal(r, 4, ax_bitmap_find(&bm, 3, 1));

	ut_assert_int_equal(r, 4, ax_bitmap_find(&bm, 4, 1));
	ut_assert_int_equal(r, 5, ax_bitmap_find(&bm, 5, 1));
	ut_assert_int_equal(r, 6, ax_bitmap_find(&bm, 6, 1));
	ut_assert_int_equal(r, 7, ax_bitmap_find(&bm, 7, 1));

	ut_assert_int_equal(r, 12, ax_bitmap_find(&bm, 8, 1));
	ut_assert_int_equal(r, 12, ax_bitmap_find(&bm, 9, 1));
	ut_assert_int_equal(r, 12, ax_bitmap_find(&bm, 10, 1));
	ut_assert_int_equal(r, 12, ax_bitmap_find(&bm, 11, 1));

	ut_assert_int_equal(r, 12, ax_bitmap_find(&bm, 12, 1));
	ut_assert_int_equal(r, 13, ax_bitmap_find(&bm, 13, 1));
	ut_assert_int_equal(r, 14, ax_bitmap_find(&bm, 14, 1));
	ut_assert_int_equal(r, 15, ax_bitmap_find(&bm, 15, 1));

	ut_assert_int_equal(r, -1, ax_bitmap_find(&bm, 16, 1));
	ut_assert_int_equal(r, -1, ax_bitmap_find(&bm, 23, 1));

	ut_assert_int_equal(r, 0, ax_bitmap_find(&bm, 0, 0));
	ut_assert_int_equal(r, 1, ax_bitmap_find(&bm, 1, 0));
	ut_assert_int_equal(r, 2, ax_bitmap_find(&bm, 2, 0));
	ut_assert_int_equal(r, 3, ax_bitmap_find(&bm, 3, 0));

	ut_assert_int_equal(r, 8, ax_bitmap_find(&bm, 4, 0));
	ut_assert_int_equal(r, 8, ax_bitmap_find(&bm, 5, 0));
	ut_assert_int_equal(r, 8, ax_bitmap_find(&bm, 6, 0));
	ut_assert_int_equal(r, 8, ax_bitmap_find(&bm, 7, 0));

	ut_assert_int_equal(r, 8, ax_bitmap_find(&bm, 8, 0));
	ut_assert_int_equal(r, 9, ax_bitmap_find(&bm, 9, 0));
	ut_assert_int_equal(r, 10, ax_bitmap_find(&bm, 10, 0));
	ut_assert_int_equal(r, 11, ax_bitmap_find(&bm, 11, 0));

	ut_assert_int_equal(r, 16, ax_bitmap_find(&bm, 12, 0));
	ut_assert_int_equal(r, 16, ax_bitmap_find(&bm, 13, 0));
	ut_assert_int_equal(r, 16, ax_bitmap_find(&bm, 14, 0));
	ut_assert_int_equal(r, 16, ax_bitmap_find(&bm, 15, 0));

	ut_assert_int_equal(r, 16, ax_bitmap_find(&bm, 16, 0));
	ut_assert_int_equal(r, 23, ax_bitmap_find(&bm, 23, 0));

}

static void bit_get(ut_runner *r)
{
	uint8_t buf[3] = { 0xF0, 0xF0, 0xF0 };
	ax_bitmap bm;
	ax_bitmap_init(&bm, buf, sizeof buf);

	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 0));
	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 1));
	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 2));
	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 3));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 4));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 5));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 6));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 7));

	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 8));
	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 9));
	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 10));
	ut_assert_int_equal(r, 0, ax_bitmap_get(&bm, 11));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 12));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 13));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 14));
	ut_assert_int_equal(r, 1, ax_bitmap_get(&bm, 15));
}

static void clear(ut_runner *r)
{
	uint8_t buf[3] = { 0, 1, 2 };
	ax_bitmap bm;
	ax_bitmap_init(&bm, buf, sizeof buf);

	ax_bitmap_clear(&bm, 1);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0xFF, 0xFF, 0xFF), 3, bm.data, 3);

	ax_bitmap_clear(&bm, 0);
	ut_assert_mem_equal(r, ax_p(uint8_t, 0, 0, 0), 3, bm.data, 3);
}

ut_suite *suite_for_bitmap()
{
	ut_suite* suite = ut_suite_create("bitmap");
	ut_suite_add(suite, clear, 0);
	ut_suite_add(suite, bit_and, 0);
	ut_suite_add(suite, bit_or, 0);
	ut_suite_add(suite, bit_not, 0);
	ut_suite_add(suite, bit_find, 0);
	ut_suite_add(suite, bit_get, 0);
	ut_suite_add(suite, bit_set, 0);
	return suite;
}
