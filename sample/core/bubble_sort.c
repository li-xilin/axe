#include "ax/flow.h"
#include "ax/vector.h"

int main()
{
	ax_vector_r v = ax_new(ax_vector, &ax_t_int);

	ax_forvalues(int, 8, 4, 7, 9, 1, 6, 5, 4, 2, 8, 1)
		ax_seq_push(v.ax_seq, &_);

	ax_iter end = ax_box_end(v.ax_box);

	ax_box_iterate(v.ax_box, i) {
		for (ax_iter j = ax_iter_ret_next(&i); !ax_iter_equal(&j, &end); ax_iter_next(&j)) {
			if (!ax_trait_less(i.etr, i.tr->get(ax_iter_c(&i)),
					j.tr->get(ax_iter_c(&j)))) {
				ax_iter_swap(&i, &j);
			}
		}
	}

	ax_dump_out(v.ax_any);
	ax_one_free(v.ax_one);
	return 0;
}

