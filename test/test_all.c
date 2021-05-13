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

#include <axut.h>
#include <ax/base.h>
#include <stdio.h>

extern axut_suite *suite_for_hmap(ax_base *base);
extern axut_suite *suite_for_vector(ax_base *base);
extern axut_suite *suite_for_string(ax_base *base);
extern axut_suite *suite_for_scope(ax_base *base);
extern axut_suite *suite_for_algo(ax_base *base);
extern axut_suite *suite_for_vail(ax_base *base);
extern axut_suite *suite_for_avl(ax_base *base);
extern axut_suite *suite_for_pool(ax_base *base);
extern axut_suite *suite_for_list(ax_base *base);
extern axut_suite *suite_for_pred(ax_base *base);
extern axut_suite *suite_for_uintk(ax_base *base);
extern axut_suite *suite_for_btrie(ax_base *base);
extern axut_suite *suite_for_seq(ax_base *base);
extern axut_suite *suite_for_stack(ax_base *base);
extern axut_suite *suite_for_queue(ax_base *base);
extern axut_suite *suite_for_arr(ax_base *base);


int main()
{
	ax_base *base = ax_base_create();

	axut_runner *r = axut_runner_create(ax_base_local(base), NULL);
	axut_runner_add(r, suite_for_seq(base));
	axut_runner_add(r, suite_for_hmap(base));
	axut_runner_add(r, suite_for_vector(base));
	axut_runner_add(r, suite_for_string(base));
	axut_runner_add(r, suite_for_scope(base));
	axut_runner_add(r, suite_for_algo(base));
	axut_runner_add(r, suite_for_avl(base));
	axut_runner_add(r, suite_for_pool(base));
	axut_runner_add(r, suite_for_list(base));
	axut_runner_add(r, suite_for_pred(base));
	axut_runner_add(r, suite_for_uintk(base));
	axut_runner_add(r, suite_for_btrie(base));
	axut_runner_add(r, suite_for_stack(base));
	axut_runner_add(r, suite_for_queue(base));
	axut_runner_add(r, suite_for_arr(base));

	axut_runner_run(r);

	fputs(axut_runner_result(r), stdout);

	ax_base_destroy(base);
}

