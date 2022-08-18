#include <ax/flow.h>
#include <stdio.h>

/* Like BASIC language for next statement.
 * FOR i = a to b STEP s
 *     ...
 * NEXT i
 */

int main(void)
{
	size_t sum = 0;

	ax_forrange(1, 101)
		sum += _;

	printf("1 + ... + 100 = %zu\n", sum);

	sum = 0;

	ax_forrange(10, 1, -2)
		sum += _;

	printf("10 + 8 + 6 + ... + 2 = %zu\n", sum);

	return 0;
}
