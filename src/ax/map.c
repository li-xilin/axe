/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
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

#include "ax/type/map.h"
#include "ax/iter.h"
#include "ax/dump.h"
#include "ax/trait.h"

const void *ax_map_key(ax_map *map, const void *key)
{
	ax_map_r self = AX_R_INIT(ax_map, map);
	ax_iter find = ax_map_at(self.ax_map, ax_trait_in(ax_class_env(map).key_tr, key)),
		end = ax_box_end(self.ax_box);
	if (ax_iter_equal(&find, &end))
		return NULL;
	return ax_map_iter_key(&find);
}

ax_dump *ax_map_dump(const ax_map *map)
{
	ax_map_cr self = ax_cr(ax_map, map);
	size_t size = ax_box_size(self.ax_box);
	ax_dump *block_dmp = NULL;

	block_dmp = ax_dump_block(ax_one_name(self.ax_one), size);
	if (!block_dmp)
		return NULL;

	const ax_trait
		*etr = ax_class_env(self.ax_box).elem_tr,
		*ktr = ax_class_env(self.ax_map).key_tr;

	size_t i = 0;
	ax_map_cforeach(self.ax_map, const void *, k, const void *, v) {

		ax_dump *pair_dmp = ax_dump_pair(
				ax_trait_dump(ktr, ax_trait_in(ktr, k)),
				ax_trait_dump(etr, ax_trait_in(etr, v))
				);

		ax_dump_bind(block_dmp, i, pair_dmp);

		i++;
	}
	
	return block_dmp;
}
