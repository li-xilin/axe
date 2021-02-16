#include <axe/iter.h>
#include <axe/box.h>

void ax_iter_swap(ax_iter it1, ax_iter it2)
{
	ax_box *box = (ax_box *)it1.owner;
	const ax_stuff_trait *tr = ax_box_elem_tr(box);
	tr->swap(ax_iter_get(it1), ax_iter_get(it2), tr->size);
}


ax_iter ax_iter_npos(void *owner, const ax_iter_trait *tr)
{
	static int dummy;
	return (ax_iter) {
		.owner = owner,
		.tr = tr,
		.point = &dummy
	};
}
