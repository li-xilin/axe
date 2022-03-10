#include "ax/flow.h"
#include "ax/vector.h"

int main()
{
	ax_vector_r v = ax_new(vector, ax_t(int));

	ax_forvalues(int, 8, 4, 7, 9, 1, 6, 5, 4, 2, 1, 1)
		ax_seq_push(v.seq, &_);

	ax_iter end = ax_box_end(v.box);

	ax_box_iterate(v.box, i) {
		for (ax_iter j = ax_iter_ret_next(&i);
				!ax_iter_equal(&j, &end); ax_iter_next(&j)) {
			if (!ax_trait_less(i.etr, i.tr->get(ax_iter_c(&i)),
					j.tr->get(ax_iter_c(&j)))) {
				ax_iter_swap(&i, &j);
			}
		}
	}

	ax_any_so(v.any);
	ax_one_free(v.one);
	return 0;
}

