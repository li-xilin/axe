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
#include "ax/def.h"
#include "ax/splay.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <stdlib.h>
#include <stdio.h>

struct number
{
        ax_splay_node node;
        int i;
};

int cmp(const ax_splay_node *left, const ax_splay_node *right)
{
        struct number *n1 = ax_container_of(left, struct number, node);
        struct number *n2 = ax_container_of(right, struct number, node);

        return n1->i - n2->i;
}

static void insert(ut_runner *r)
{
	ax_splay t;
	int i = 0;
        ax_splay_init(&t, cmp);

        for (int i = 0; i < 100; i++) {
                struct number *p = malloc(sizeof *p);
                p->i = i;
                ax_splay_insert(&t, &p->node);
        }

	i = 0;
        ax_splay_node *cur = ax_splay_first(&t);
        while (cur) {
                struct number *p = ax_container_of(cur, struct number, node);
		ut_assert_int_equal(r, i, p->i);
                cur = ax_splay_next(cur);
		i++;
        }

        for (int i = 0; i < 100; i += 2) {
                struct number n = { .i = i };
                ax_splay_node *node = ax_splay_find(&t, &n.node);
                struct number *p = ax_container_of(node, struct number, node);
                ax_splay_remove(&t, &p->node);
        }

	i = 1;
        cur = ax_splay_first(&t);
        while (cur) {
                struct number *p = ax_container_of(cur, struct number, node);
		ut_assert_int_equal(r, i, p->i);
                cur = ax_splay_next(cur);
		i += 2;
        }

	i = 99;
        cur = ax_splay_last(&t);
        while (cur) {
                struct number *p = ax_container_of(cur, struct number, node);
		ut_assert_int_equal(r, i, p->i);
                cur = ax_splay_prev(cur);
		i -= 2;
        }
}

ut_suite *suite_for_splay()
{
	ut_suite* suite = ut_suite_create("splay");
	ut_suite_add(suite, insert, 0);
	return suite;
}

