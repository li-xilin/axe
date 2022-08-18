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

#include "ut/runner.h"

#include <stdio.h>

extern ut_suite *suite_for_hmap();
extern ut_suite *suite_for_vector();
extern ut_suite *suite_for_string();
extern ut_suite *suite_for_algo();
extern ut_suite *suite_for_vail();
extern ut_suite *suite_for_avl();
extern ut_suite *suite_for_list();
extern ut_suite *suite_for_pred();
extern ut_suite *suite_for_u1024();
extern ut_suite *suite_for_btrie();
extern ut_suite *suite_for_seq();
extern ut_suite *suite_for_stack();
extern ut_suite *suite_for_queue();
extern ut_suite *suite_for_arr();
extern ut_suite *suite_for_mem();
extern ut_suite *suite_for_class();
extern ut_suite *suite_for_stuff();
extern void suite_for_maps(ut_runner *r);

int main()
{
	ut_runner *r = ax_new(ut_runner, NULL).ut_runner;

	ut_runner_add(r, suite_for_seq());
	ut_runner_add(r, suite_for_hmap());
	ut_runner_add(r, suite_for_vector());
	ut_runner_add(r, suite_for_string());
	ut_runner_add(r, suite_for_algo());
	ut_runner_add(r, suite_for_avl());
	ut_runner_add(r, suite_for_list());
	ut_runner_add(r, suite_for_pred());
	ut_runner_add(r, suite_for_u1024());
	ut_runner_add(r, suite_for_btrie());
	ut_runner_add(r, suite_for_stack());
	ut_runner_add(r, suite_for_queue());
	ut_runner_add(r, suite_for_arr());
	ut_runner_add(r, suite_for_mem());
	ut_runner_add(r, suite_for_class());
	ut_runner_add(r, suite_for_stuff());

	suite_for_maps(r);

	ut_runner_run(r);
	fputs(ut_runner_result(r), stdout);

	ax_one_free(ax_r(ut_runner, r).ax_one);
	return 0;
}

