#include "axe/uintk.h"
#include "axe/base.h"
#include "axe/stuff.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

void factorial(ax_uintk* n, ax_uintk* res)
{
	ax_uintk tmp;
	ax_uintk_assign(&tmp, n);

	ax_uintk_dec(n);

	while (!ax_uintk_is_zero(n)) {
		ax_uintk_mul(&tmp, n, res);
		ax_uintk_dec(n);

		ax_uintk_assign(&tmp, res);
	}

	ax_uintk_assign(res, &tmp);
}



static void test_factorial(axut_runner *r)
{
	char resbuf[512];
	char expect[] =
		"1b30964ec395dc24069528d54bbda40d16e966ef9a70eb21b5b294"
		"3a321cdf10391745570cca9420c6ecb3b72ed2ee8b02ea2735c61a"
		"000000000000000000000000";
	ax_uintk num = { 0 };
	ax_uintk res = { 0 };
	ax_uintk_from_int(&num, 100);
	factorial(&num, &res);
	ax_uintk_to_string(&res, resbuf, sizeof resbuf);
	axut_assert(r, strcmp(expect, resbuf) == 0);


}
axut_suite *suite_for_uintk(ax_base *base)
{
	axut_suite* suite = axut_suite_create(ax_base_local(base), "uintk");

	axut_suite_add(suite, test_factorial, 0);

	return suite;
}
