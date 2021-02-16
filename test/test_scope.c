#include "axe/scope.h"
#include "axe/iter.h"
#include "axe/vector.h"
#include "axe/base.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static void test_scope_create(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_scope_role role = ax_scope_create(ax_base_local(base));
	axut_assert(r, role.one != NULL);
	ax_scope_destroy(role.scope);
	ax_base_destroy(base);
}

static void test_scope_global(axut_runner *r)
{
	ax_base* base = ax_base_create();
	ax_vector_role role0 = ax_vector_create(ax_base_global(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_role role1 = ax_vector_create(ax_base_global(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_role role2 = ax_vector_create(ax_base_global(base), ax_stuff_traits(AX_ST_I32));
	ax_base_destroy(base);
}

static void test_scope_local(axut_runner *r)
{
	ax_base* base = ax_base_create();
	assert(base);
	ax_vector_role v1_role = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	{
		int d = ax_base_enter(base);
		ax_vector_role v2_role = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
		{
			int d = ax_base_enter(base);
			ax_vector_role v3_role = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
			ax_base_leave(base, d);
		}

		ax_base_leave(base, d);
	}
	ax_vector_role v4_role = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_vector_role v5_role = ax_vector_create(ax_base_local(base), ax_stuff_traits(AX_ST_I32));
	ax_base_destroy(base);
}


axut_suite *suite_for_scope(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "scope");

	axut_suite_add(suite, test_scope_create, 0);
	axut_suite_add(suite, test_scope_global, 0);
	axut_suite_add(suite, test_scope_local, 0);

	return suite;
}
