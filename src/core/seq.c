/*
 * Copyright (c) 2020 - 2021 Li hsilin <lihsilyn@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "ax/type/seq.h"
#include "ax/dump.h"
#include "ax/trait.h"
#include "ax/arraya.h"
#include "check.h"

#include <errno.h>

size_t ax_seq_array(ax_seq *seq, void *elems[], size_t len)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(elems);

	ax_seq_r self = AX_R_INIT(ax_seq, seq);
	ax_iter pos = ax_box_rbegin(self.ax_box),
		end = ax_box_rend(self.ax_box);

	size_t i;
	for (i = 0; i < len && !ax_iter_equal(&pos, &end); i++) {
		elems[i] = ax_iter_get(&pos);
		ax_iter_next(&pos);
	}
	return i; 
}

ax_dump *ax_seq_dump(const ax_seq *seq)
{
	CHECK_PARAM_NULL(seq);

	ax_seq_cr self = AX_R_INIT(ax_seq, seq);
	size_t size = ax_box_size(self.ax_box);
	ax_dump *block_dmp = ax_dump_block(ax_one_name(self.ax_one), size);
	const ax_trait *etr = ax_class_data(self.ax_box).elem_tr;
	size_t i = 0;
	ax_box_cforeach(self.ax_box, const void *, p) {
		ax_dump_bind(block_dmp, i, ax_trait_dump(etr, ax_trait_in(etr, p)));
		i++;
	}
	return block_dmp;
}

ax_fail ax_seq_push_arraya(ax_seq *seq, const void *arrp)
{
	CHECK_PARAM_NULL(seq);
	CHECK_PARAM_NULL(arrp);

	ax_seq_cr self = AX_R_INIT(ax_seq, seq);
	size_t size = ax_arraya_size(arrp);
	int esize = ax_trait_size(ax_class_data(self.ax_box).elem_tr);
	ax_assert(size % esize == 0,
			"different size of elements with seq and arraya");
	const ax_byte *p = arrp;
	size_t i;
	for (i = 0; i < size / esize; i++) {
		if (seq->tr->push(seq, p + esize * i, NULL))
			goto fail;
	}
	return false;
fail:
	for (size_t j = 0; j < i; j++)
		seq->tr->pop(seq);
	return true;;
}
