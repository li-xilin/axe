#include "ax/deq.h"
#include <stdio.h>

int main()
{
	// Initialize de-queue
	ax_deq_r d = ax_new(deq, ax_t(int));
	ax_forrange(0, 9) {
		ax_seq_pushf(d.seq, ax_p(int, _));
		ax_seq_push(d.seq, ax_p(int, _));
	}

	// Random access
	ax_forrange(0, ax_box_size(d.box)) {
		ax_iter it = ax_seq_at(d.seq, _);
		printf("%d ", *(int *)ax_iter_get(&it));
	}
	putchar('\n');


	// Iterate
	ax_box_foreach(d.box, int *, a)
		*a += 1;
	ax_box_foreach(d.box, int *, a)
		printf("%d ", *a);
	putchar('\n');

	ax_one_free(d.one);

	return 0;
}
