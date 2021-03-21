#ifndef AXE_PRED_H_
#define AXE_PRED_H_
#include "debug.h"
#include "def.h"

#define AX_BIND_1 1
#define AX_BIND_2 2
#define AX_BIND_U 4

#ifndef AX_PRED_DEFINED
#define AX_PRED_DEFINED
typedef struct ax_pred_st ax_pred;
#endif

struct ax_pred_st
{
	union {
		ax_unary_f u;
		ax_binary_f b;
	} fun;
	void *first;
	void *second;
	void *args;
	int   bind;
};

inline static ax_pred ax_pred_unary_make(ax_unary_f oper, void *in, void *args)
{
	return (ax_pred) {
		.fun.u = oper,
		.first = in,
		.second = NULL,
		.bind = (in ? AX_BIND_1 : 0) | AX_BIND_U,
		.args = args
	};
}

inline static ax_pred ax_pred_binary_make(ax_binary_f oper, void *in1, void *in2, void *args)
{
	return (ax_pred) {
		.fun.b = oper,
		.first = in1,
		.second = in2,
		.bind = (in1 ? (in2 ? (AX_BIND_1|AX_BIND_2) : AX_BIND_1) : (in2 ? AX_BIND_2 : 0 )),
		.args = args
	};
}

inline static void ax_pred_do(const ax_pred *pred, void *out, const void *in1, const void *in2)
{
#ifdef AX_DEBUG
	const char *failed_msg = "invalid argument for predicate input";
#endif
	switch (pred->bind) {
		case AX_BIND_U:
			ax_assert(!in1 && !in2, failed_msg);
			goto do_unary;
		case AX_BIND_U | AX_BIND_1:
			in1 = pred->first;
			ax_assert(in1 && !in2, failed_msg);
			goto do_unary;
		case 0:
			ax_assert(in1 && in2, failed_msg);
			goto do_binary;
		case AX_BIND_1:
			ax_assert(in1 && !in2, failed_msg);
			in2 = in1;
			in1 = pred->first;
			goto do_binary;
		case AX_BIND_2:
			ax_assert(in1 && !in2, failed_msg);
			in2 = pred->second;
			goto do_binary;
		case AX_BIND_1 | AX_BIND_2:
			ax_assert(!in1 && !in2, failed_msg);
			in1 = pred->first;
			in2 = pred->second;
			goto do_binary;

	}
do_unary:
	pred->fun.u(out, in1, pred->args);
	return;
do_binary:
	pred->fun.b(out, in1, in2, pred->args);
}

#endif
