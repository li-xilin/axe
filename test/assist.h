#ifndef UTIL_H_
#define UTIL_H_
#include <axe/algo.h>
#include <axe/seq.h>

static ax_bool seq_equal_array(ax_seq *seq, void *arr, size_t mem_size)
{
	 return ax_equal_to_arr(ax_box_begin(&seq->__box), ax_box_end(&seq->__box), arr, mem_size);
}
#endif
