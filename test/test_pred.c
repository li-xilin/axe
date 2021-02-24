#include "axe/pred.h"
#include "axe/oper.h"

#include "axut.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>

static void pred_unary(axut_runner *r)
{
	uint32_t in = 1, out;
	ax_pred pred = ax_pred_unary_make(ax_oper_for(AX_ST_U32)->bit_not, NULL, NULL);
	ax_pred_do(&pred, &out, &in, NULL);
	axut_assert(r, out == 0xFFFFFFFE);

	ax_bool bool_out;
	pred = ax_pred_unary_make(ax_oper_for(AX_ST_U32)->not, &in, NULL);
	ax_pred_do(&pred, &bool_out, NULL, NULL);
	axut_assert(r, bool_out == ax_false);

}

static void pred_binary(axut_runner *r)
{
	ax_pred pred;
	uint32_t in1 = 3, in2 = 4, out;

	pred = ax_pred_binary_make(ax_oper_for(AX_ST_U32)->add, NULL, NULL, NULL);
	ax_pred_do(&pred, &out, &in1, &in2);
	axut_assert(r, out == 7);

	pred = ax_pred_binary_make(ax_oper_for(AX_ST_U32)->add, &in1, NULL, NULL);
	ax_pred_do(&pred, &out, &in2, NULL);
	axut_assert(r, out == 7);

	pred = ax_pred_binary_make(ax_oper_for(AX_ST_U32)->add, NULL, &in2, NULL);
	ax_pred_do(&pred, &out, &in1, NULL);
	axut_assert(r, out == 7);

	pred = ax_pred_binary_make(ax_oper_for(AX_ST_U32)->add, &in1, &in2, NULL);
	ax_pred_do(&pred, &out, NULL, NULL);
	axut_assert(r, out == 7);

}

axut_suite *suite_for_pred(ax_base *base)
{
	axut_suite *suite = axut_suite_create(ax_base_local(base), "pred");

	axut_suite_add(suite, pred_unary, 0);
	axut_suite_add(suite, pred_binary, 0);

	return suite;
}
