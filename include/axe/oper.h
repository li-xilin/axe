#ifndef AXE_OPER_H_
#define AXE_OPER_H_
#include "stuff.h"
#include "def.h"

typedef struct ax_operset_st ax_operset;

struct ax_operset_st
{
    ax_binary_f add;
    ax_binary_f sub;
    ax_binary_f mul;
    ax_binary_f div;
    ax_binary_f mod;

    ax_binary_f and;
    ax_binary_f or;
    ax_unary_f  not;

    ax_binary_f bit_and;
    ax_binary_f bit_or;
    ax_unary_f  bit_not;
    ax_binary_f bit_xor;

    ax_binary_f gt;
    ax_binary_f ge;
    ax_binary_f lt;
    ax_binary_f le;
    ax_binary_f eq;
    ax_binary_f ne;

    ax_unary_f hash;
};

const ax_operset *ax_oper_for(int type);

#endif
