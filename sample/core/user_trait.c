#include <ax/vector.h>
#include <ax/dump.h>
#include <ax/mem.h>
#include <ax/trait.h>
#include <ax/algo.h>
#include <stdio.h>
#include <stdlib.h>

struct stuff
{
	unsigned int id;
	char *name;
	char phone[12];
};

ax_fail stuff_trait_copy(struct stuff *dst, const struct stuff *src, size_t size)
{
	dst->id = src->id;
	memcpy(dst->phone, src->phone, sizeof src->phone);
	dst->name = ax_strdup(src->name);
	if (!dst->name)
		return true;
	return false;
}

ax_fail stuff_trait_init(struct stuff *stuff, va_list *ap)
{
	ax_assert(ap, "struct stuff have no default value");
	stuff->id = va_arg(*ap, unsigned int);
	stuff->name = va_arg(*ap, char *);
	if (!stuff->name)
		return true;
	memcpy(stuff->phone, va_arg(*ap, char *), sizeof stuff->phone);
	return false;
}

void stuff_trait_free(struct stuff *stuff)
{
	free(stuff->name);
}

ax_dump* stuff_trait_dump(const struct stuff *stuff, size_t size)
{
	ax_dump *dump = ax_dump_block("stuff", 3);
	ax_dump_bind(dump, 0, ax_dump_uint(stuff->id));
	ax_dump_bind(dump, 1, ax_dump_str(stuff->name));
	ax_dump_bind(dump, 2, ax_dump_str(stuff->phone));
	return dump;
}

ax_trait_declare(stuff, struct stuff);

ax_trait_define(stuff,
	COPY((ax_trait_copy_f) stuff_trait_copy),
	INIT((ax_trait_init_f) stuff_trait_init),
	FREE((ax_trait_free_f) stuff_trait_free),
	DUMP((ax_trait_dump_f) stuff_trait_dump));

int main()
{
	ax_vector_r stuffs = ax_new(ax_vector, ax_t(stuff));

	ax_seq_ipush(stuffs.ax_seq, 1, "Zhang San", "15200000001");
	ax_seq_ipush(stuffs.ax_seq, 2, "Li Si", "15200000002");
	ax_seq_ipush(stuffs.ax_seq, 3, "Wang Wu", "15200000003");

	struct stuff zhaoliu = { .id = 4, .name = "Zhao Liu", .phone = "15200000004", };
	ax_seq_push(stuffs.ax_seq, &zhaoliu);

	ax_any_so(stuffs.ax_any);

	return 0;
}

