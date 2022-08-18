#include "ax/array.h"
#include <stdio.h>
#include <inttypes.h>

int main()
{
	char arr[] = "Hello world";

	ax_array_r a = ax_new(ax_array, ax_t(char), arr, sizeof(arr));
	printf("size = %zu\n", ax_box_size(a.ax_box));
	printf("maxsize = %zu\n", ax_box_maxsize(a.ax_box));

	ax_forrange(0, ax_box_size(a.ax_box)) {
		ax_iter it = ax_seq_at(a.ax_seq, _);
		printf("%c ", *(char *)ax_iter_get(&it));
	}
	putchar('\n');

	return 0;
}
