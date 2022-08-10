#include <ax/flow.h>
#include <stdio.h>

int main(void)
{
	ax_forvalues(int, 3, 1, 4, 1, 5, 9, 2, 6, 5, 3)
		printf("%d ", _);
	putchar('\n');

	typedef struct { int bar; } foo;
	ax_forvalues(foo,
			*ax_p(foo, { 1 }),
			*ax_p(foo, { 2 }),
			*ax_p(foo, { 3 }),
			)
		printf("%d ", _.bar);
	putchar('\n');

	return 0;
}
