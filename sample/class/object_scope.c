#include "ax/string.h"
#include "ax/ptra.h"
#include "ax/trait.h"
#include <stdio.h>
#include <stdlib.h>

int main()
{
	uint32_t *p1 = malloc(sizeof *p1);
	float *p2 = malloc(sizeof *p2);
	ax_string_r s = ax_new0(string);
	ax_str_append(s.str, "a test string");

	*p1 = 1;
	*p2 = 2.5;

	ax_scope (ax_onelize(p1), ax_onelize(p2), s.one) {
		printf("p1 = %d\n", *p1);
		printf("p2 = %f\n", *p2);
		printf("str = %s\n", ax_str_strz(s.str));
	}

	return 0;
}
