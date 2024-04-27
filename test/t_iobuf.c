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

#include "assist.h"
#include "ax/iobuf.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <stdlib.h>
#include <stdio.h>

static void pullup(ut_runner *r)
{
	{
		char buffer[16];
		ax_iobuf b;
		ax_iobuf_init(&b, buffer,sizeof buffer);
		ax_iobuf_write(&b, "hello", 5);
		ax_iobuf_write(&b, " world", 6);
		char *act = ax_iobuf_pullup(&b);
		char *exp = "hello world";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = " abcdefghij                                          ";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 1, .rear = 11, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghij";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = "                                           abcdefghij";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 43, .rear = 0, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghij";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = "klmnop                                     abcdefghij";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 43, .rear = 6, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghijklmnop";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = "klmnopqrstuvwxyz0123456789abcdefjhijklmn   abcdefghij";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 43, .rear = 40, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghijklmnopqrstuvwxyz0123456789abcdefjhijklmn";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = "efjhijklmn   abcdefghijklmnopqrstuvwxyz0123456789abcd";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 13, .rear = 10, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghijklmnopqrstuvwxyz0123456789abcdefjhijklmn";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = "klmnopqrstuvwxyz0123456789abcdefjhijklmnop abcdefghij";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 43, .rear = 42, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghijklmnopqrstuvwxyz0123456789abcdefjhijklmnop";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}


	{
		char buffer[] = "efjhijklmnop abcdefghijklmnopqrstuvwxyz0123456789abcd";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 13, .rear = 12, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghijklmnopqrstuvwxyz0123456789abcdefjhijklmnop";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = "klmnopqrstuvwxyz0123456789                 abcdefghij";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 43, .rear = 26, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "abcdefghijklmnopqrstuvwxyz0123456789";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));
	}

	{
		char buffer[] = "efjhijklmnop                           0123456789abcd";
		ax_iobuf b = { .buf = (uint8_t *)buffer, .front = 39, .rear = 12, .size = 53 };
		char *act = ax_iobuf_pullup(&b);
		char *exp = "0123456789abcdefjhijklmnop";
		ut_assert_mem_equal(r, exp, strlen(exp), act, ax_iobuf_data_size(&b));

	}

}



ut_suite *suite_for_iobuf()
{
	ut_suite* suite = ut_suite_create("iobuf");
	ut_suite_add(suite, pullup, 0);
	return suite;
}

