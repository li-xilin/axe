#include "ax/string.h"
#include "ax/algo.h"
#include <ctype.h>

void to_upper(void *out, const void *in, void *arg)
{
	*(char *)out = toupper(*(char *)in);
}

int main()
{
	ax_string_r s = ax_new0(string);
	ax_str_append(s.str, "Hello world");

	ax_pred pred = ax_pred_unary_make(to_upper, NULL, NULL);

	ax_iter cur = ax_box_begin(s.box),
		end = ax_box_end(s.box);

	ax_transform(ax_iter_c(&cur), ax_iter_c(&end), &cur, &pred);
	ax_any_so(s.any);

	return 0;
}

