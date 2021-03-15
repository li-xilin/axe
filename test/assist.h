#ifndef UTIL_H_
#define UTIL_H_
#include <axe/algo.h>
#include <axe/seq.h>

static ax_bool seq_equal_array(ax_seq *seq, void *arr, size_t mem_size)
{
	 return ax_equal_to_arr(ax_box_begin(&seq->_box), ax_box_end(&seq->_box), arr, mem_size);
}
#endif
