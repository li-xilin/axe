#include "ax/trick.h"
#include "ax/narg.h"
#include "ax/def.h"
#include <stdio.h>

#define show(line) printf("%s -> %s\n", #line, ax_stringy(line))

int main()
{
	show(AX_INC(3));
	show(AX_DEC(3));

#define FUN1(i, x) fun1(i, x)
	show(AX_TRANSFORM(FUN1, a, b, c, d, e, f));


#define FUN2(n, ...) fun2(n, __VA_ARGS__)
	show(AX_PAVE_TO(3, FUN2, args...));

#define baseof_A B
#define baseof_B C
#define baseof_C D
#define baseof_D E
#define baseof_E F
	show(AX_EXPAND_PREFIX(5, baseof_, A));

#define FUN3_1(a) fun3_1(a)
#define FUN3_2(a, b) fun3_2(a, b)
#define FUN3_3(a, b, c) fun3_3(a, b, c)
show(AX_OVERLOAD(FUN3_, 1));
show(AX_OVERLOAD(FUN3_, 1, 2));
show(AX_OVERLOAD(FUN3_, 1, 2, 3));

show(AX_CATENATE(_, a, b, c, d, e, f, g, h ,i));

show(ax_sizeof(char, short, int, long));

}
