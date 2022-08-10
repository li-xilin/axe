#include "ax/array.h"
#include "ax/trait.h"
#include "ax/algo.h"
#include <stdio.h>

bool str_less(const char *a, const char *b, size_t size)
{
	int diff = strlen(a) - strlen(b);
	return diff ? diff < 0 : strcmp(a, b) < 0;
}

int main()
{
	/* setup string trait for sorting */
	ax_trait str_tr = *ax_t(str);
	str_tr.less = (ax_trait_compare_f)str_less;

	char *strs[] = {
		"Amy",
		"Tony",
		"Alice",
		"Bob",
		"Alex",
		"Oscar",
		"Jennifer",
		"Ed"
	};

	/* alloc an ax_arr instance and initialize it with specified array 
	   ax_arr need not be freed */
	ax_array_r arr = ax_new(ax_array, &str_tr, strs, sizeof(strs));
	
	/* sort the sequence in ascending order*/
	ax_quick_sort(ax_ptrof(ax_iter, ax_box_begin(arr.ax_box)), 
			ax_ptrof(ax_iter, ax_box_end(arr.ax_box)));

	/* reverse the sequence */
	ax_seq_invert(arr.ax_seq);

	/* output the result */
	ax_box_foreach(arr.ax_box, char *, s)
		printf("%s\n", s);

	return 0;
}

