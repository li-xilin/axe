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
#include "ax/iter.h"
#include "ax/vector.h"
#include "ax/list.h"
#include "ax/algo.h"

#include "axut.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


static void push_arraya(ax_runner *r)
{
	ax_list_r list_r = ax_new(list, ax_t(int));
	ax_seq_push_arraya(list_r.seq, ax_arraya(int, 1, 2, 3, 4, 5));
	int arr[] = { 1, 2 ,3 ,4, 5 };
	seq_equal_array(list_r.seq, arr, sizeof(arr));
	ax_one_free(list_r.one);
}


axut_suite *suite_for_seq()
{
	axut_suite* suite = axut_suite_create("seq");

	axut_suite_add(suite, push_arraya, 0);
	return suite;
}
