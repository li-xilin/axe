/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
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

#include <ax/mem.h>
#include <ax/iter.h>
#include <ax/box.h>

void ax_iter_swap(const ax_iter *it1, const ax_iter *it2)
{
	//TODO: check same owner
	ax_box *box = ax_iter_box(it1);
	const ax_stuff_trait *tr = ax_box_elem_tr(box);
	ax_mem_swap(it1->tr->get(it1), it2->tr->get(it2), tr->size);
}

void *ax_iter_get(const ax_iter *it)
{
	ax_box *box = (ax_box *)it->owner;
	const ax_stuff_trait *tr = ax_box_elem_tr(box);
	return tr->link
	       ? *(void **)it->tr->get(it)
	       : it->tr->get(it);
}

ax_citer ax_citer_npos(const ax_citer *it)
{
	static int dummy;
	return (ax_citer) {
		.owner = it->owner,
		.tr = it->tr,
		.point = &dummy
	};
}
