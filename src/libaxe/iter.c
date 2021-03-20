#include <axe/iter.h>
#include <axe/box.h>

void ax_iter_swap(const ax_iter *it1, const ax_iter *it2)
{
	ax_box *box = (ax_box *)it1->owner;
	const ax_stuff_trait *tr = ax_box_elem_tr(box);
	tr->swap(ax_iter_get(it1),
			ax_iter_get(it2),
			tr->size);
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
