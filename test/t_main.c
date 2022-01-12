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
#include <stdio.h>

extern axut_suite *suite_for_hmap();
extern axut_suite *suite_for_vector();
extern axut_suite *suite_for_string();
extern axut_suite *suite_for_algo();
extern axut_suite *suite_for_vail();
extern axut_suite *suite_for_avl();
extern axut_suite *suite_for_list();
extern axut_suite *suite_for_pred();
extern axut_suite *suite_for_uintk();
extern axut_suite *suite_for_btrie();
extern axut_suite *suite_for_seq();
extern axut_suite *suite_for_stack();
extern axut_suite *suite_for_queue();
extern axut_suite *suite_for_arr();
extern axut_suite *suite_for_mem();
extern axut_suite *suite_for_class();
extern axut_suite *suite_for_stuff();
extern void suite_for_maps(axut_runner *r);

int main()
{
	axut_runner *r = axut_runner_create(NULL);

	axut_runner_add(r, suite_for_seq());
	axut_runner_add(r, suite_for_hmap());
	axut_runner_add(r, suite_for_vector());
	axut_runner_add(r, suite_for_string());
	axut_runner_add(r, suite_for_algo());
	axut_runner_add(r, suite_for_avl());
	axut_runner_add(r, suite_for_list());
	axut_runner_add(r, suite_for_pred());
	axut_runner_add(r, suite_for_uintk());
	axut_runner_add(r, suite_for_btrie());
	axut_runner_add(r, suite_for_stack());
	axut_runner_add(r, suite_for_queue());
	axut_runner_add(r, suite_for_arr());
	axut_runner_add(r, suite_for_mem());
	axut_runner_add(r, suite_for_class());
	axut_runner_add(r, suite_for_stuff());

	suite_for_maps(r);

	axut_runner_run(r);
	fputs(axut_runner_result(r), stdout);

	axut_runner_destroy(r);
	return 0;
}

