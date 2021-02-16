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

static void test_string_create(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_string_role role = ax_string_create(ax_base_local(base));
	axut_assert(r, role.any != NULL);
	axut_assert(r, ax_box_size(role.box) == 0);
	axut_assert(r, ax_str_length(role.str) == 0);
	ax_base_destroy(base);
}

static void test_string_complex(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_string_role role = ax_string_create(ax_base_local(base));
	ax_str_append(role.str, "hello");
	ax_str_append(role.str, " world");
	axut_assert(r, strcmp(ax_str_cstr(role.str), "hello world") == 0);
	axut_assert(r, ax_str_length(role.str) == sizeof "hello world" - 1);
	ax_str_insert(role.str, 6, "my ");
	axut_assert(r, strcmp(ax_str_cstr(role.str), "hello my world") == 0);

	ax_base_destroy(base);
}

static void test_string_split(axut_runner *r)
{
	ax_base* base = ax_base_create();
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

	ax_base_destroy(base);
}

axut_suite *suite_for_string(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "string");

	axut_suite_add(suite, test_string_create, 0);
	axut_suite_add(suite, test_string_complex, 0);
	axut_suite_add(suite, test_string_split, 0);

	return suite;
}
