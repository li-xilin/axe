/*
 * Copyright (c) 2024 Li Xilin <lixilin@gmx.com>
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

#include "check.h"
#include "ax/lock.h"
#include <stdlib.h>

static int lock_init_default(void *arg)
{
	*(int *)arg = 0;
	return 0;
}

static int lock_get_default(void *arg)
{
	while (*(int *)arg == 0);
	return 0;
}

static int lock_set_default(void *arg)
{
	*(int *)arg = 1;
	return 0;
}

static void lock_free_default(void *arg)
{
	free(arg);
}

static const ax_lock_trait s_def_trait = {
	.arg_size = sizeof(int),
	.t_init = lock_init_default,
	.t_get = lock_get_default,
	.t_put = lock_set_default,
	.t_free = lock_free_default,
};

static const ax_lock_trait *s_cur_trait = &s_def_trait;

void ax_lock_set_trait(const ax_lock_trait *t)
{
	if (t) {
		CHECK_PARAM_NULL(t->t_init);
		CHECK_PARAM_NULL(t->t_get);
		CHECK_PARAM_NULL(t->t_put);
		CHECK_PARAM_NULL(t->t_free);
	}
	s_cur_trait = t ? t : &s_def_trait;
}

int ax_lock_init(ax_lock *l)
{
	void *arg = malloc(s_cur_trait->arg_size);
	if (!arg)
		return -1;

	if (s_cur_trait->t_init(arg)) {
		free(arg);
		return -1;
	}
	l->l_arg = arg;
	l->l_tr = s_cur_trait;
	return 0;
}

