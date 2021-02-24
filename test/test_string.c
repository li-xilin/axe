#include "axe/iter.h"
#include "axe/string.h"
#include "axe/seq.h"
#include "axe/base.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void create(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_string_role role = ax_string_create(ax_base_local(base));
	axut_assert(r, role.any != NULL);
	axut_assert(r, ax_box_size(role.box) == 0);
	axut_assert(r, ax_str_length(role.str) == 0);
}

static void append(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_string_role role = ax_string_create(ax_base_local(base));
	ax_str_append(role.str, "hello");
	ax_str_append(role.str, " world");
	axut_assert(r, strcmp(ax_str_cstr(role.str), "hello world") == 0);
	axut_assert(r, ax_str_length(role.str) == sizeof "hello world" - 1);
	ax_str_insert(role.str, 6, "my ");
	axut_assert(r, strcmp(ax_str_cstr(role.str), "hello my world") == 0);
}

static void split(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_string_role role = ax_string_create(ax_base_local(base));
	ax_str_append(role.str, ":111:222::");
	ax_seq *ret = ax_str_split(role.str, ':');
	

	int i = 0;
	ax_foreach(char*, p, &ret->box) {
		switch(i) {
			case 0: axut_assert_str_equal(r, p, ""); break;
			case 1: axut_assert_str_equal(r, p, "111"); break;
			case 2: axut_assert_str_equal(r, p, "222"); break;
			case 3: axut_assert_str_equal(r, p, ""); break;
			case 4: axut_assert_str_equal(r, p, ""); break;
		}
		i++;
	}

}

static void cleanup(axut_runner *r)
{
	ax_base *base = axut_runner_arg(r);
	ax_base_destroy(base);
}

axut_suite *suite_for_string(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "string");

	ax_base* sbase = ax_base_create();
	axut_suite_set_arg(suite, sbase);

	axut_suite_add(suite, create, 0);
	axut_suite_add(suite, append, 0);
	axut_suite_add(suite, split, 0);
	axut_suite_add(suite, cleanup, 0xFF);

	return suite;
}
