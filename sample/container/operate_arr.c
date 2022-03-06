#include "ax/array.h"
#include "ax/box.h"
#include <stdio.h>
#include <inttypes.h>

int main()
{
	char arr[] = "Hello world";

	ax_array_r a = ax_class_new(array, ax_t(char), arr, sizeof(arr));
	printf("size = %zu\n", ax_box_size(a.box));
	printf("maxsize = %zu\n", ax_box_maxsize(a.box));

	ax_forrange(0, ax_box_size(a.box)) {
		ax_iter it = ax_seq_at(a.seq, _);
		printf("%c ", *(char *)ax_iter_get(&it));
	}
	putchar('\n');

	return 0;
}
