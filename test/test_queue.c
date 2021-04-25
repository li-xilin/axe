#include "axe/queue.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_queue_r queue = ax_queue_create(ax_base_local(base), ax_stuff_traits(AX_ST_I));
	axut_assert_uint_equal(r, 0, ax_tube_size(queue.tube));
}

static void operate(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_queue_r queue = ax_queue_create(ax_base_local(base), ax_stuff_traits(AX_ST_I));
	int value;

	value = 1;
	ax_tube_push(queue.tube, &value);
	axut_assert_int_equal(r, 1, *(int *)ax_tube_prime(queue.tube));
	axut_assert_uint_equal(r, 1, ax_tube_size(queue.tube));

	value = 2;
	ax_tube_push(queue.tube, &value);
	axut_assert_int_equal(r, 1, *(int *)ax_tube_prime(queue.tube));
	axut_assert_uint_equal(r, 2, ax_tube_size(queue.tube));

	ax_tube_pop(queue.tube);
	axut_assert_int_equal(r, 2, *(int *)ax_tube_prime(queue.tube));
	axut_assert_uint_equal(r, 1, ax_tube_size(queue.tube));

	ax_tube_pop(queue.tube);
	axut_assert_uint_equal(r, 0, ax_tube_size(queue.tube));
}

static void clean(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_base_destroy(base);
}

axut_suite *suite_for_queue(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "queue");

	ax_base *base1 = ax_base_create();
	axut_suite_set_arg(suite, base1);

	axut_suite_add(suite, create, 0);
	axut_suite_add(suite, operate, 1);

	axut_suite_add(suite, clean, 0xFF);
	return suite;
}
