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

#include "ax/u1024.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void factorial(ax_u1024* n, ax_u1024* res)
{
	ax_u1024 tmp;
	ax_u1024_assign(&tmp, n);

	ax_u1024_dec(n);

	while (!ax_u1024_is_zero(n)) {
		ax_u1024_mul(&tmp, n, res);
		ax_u1024_dec(n);

		ax_u1024_assign(&tmp, res);
	}

	ax_u1024_assign(res, &tmp);
}



static void test_factorial(ut_runner *r)
{
	char resbuf[512];
	char expect[] =
		"1b30964ec395dc24069528d54bbda40d16e966ef9a70eb21b5b294"
		"3a321cdf10391745570cca9420c6ecb3b72ed2ee8b02ea2735c61a"
		"000000000000000000000000";
	ax_u1024 num = { 0 };
	ax_u1024 res = { 0 };
	ax_u1024_from_int(&num, 100);
	factorial(&num, &res);
	ax_u1024_to_string(&res, resbuf, sizeof resbuf);
	ut_assert(r, strcmp(expect, resbuf) == 0);


}
ut_suite *suite_for_u1024()
{
	ut_suite* suite = ut_suite_create("u1024");

	ut_suite_add(suite, test_factorial, 0);

	return suite;
}
