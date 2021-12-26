#include <ax/dump.h>
#include <ax/mem.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>

static int snumber(int64_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%" PRIi64, value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int unumber(uint64_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%" PRIu64, value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int fnumber(double value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%lg", value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int pointer(const void *value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%p", value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int string(const char *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret = sprintf(buf, "%p \"", (void *)value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	
	if ((ret = out_cb(value, length, ctx)))
		return ret;
	strcpy(buf, "\"");
	if ((ret = out_cb(buf, 1, ctx)))
		return ret;

	return 0;
}

static int wstring(const wchar_t *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret = sprintf(buf, "%p L\"", (void *)value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	{
		mbstate_t mbs = { 0 };
		const wchar_t *p = (void *)value;
		size_t conv = 0;
		buf[sizeof(buf) - 1] = '\0';
		while (p && (conv = wcsrtombs(buf, &p, sizeof(buf) - 1, &mbs)) != 0)
			if ((ret = out_cb(buf, conv, ctx)))
				return ret;
		if (conv == (size_t)-1) 
			if ((ret = out_cb("BADCHAR", sizeof "BADCHAR" -1, ctx)))
				return ret;
	}
	strcpy(buf, "\"");
	if ((ret = out_cb(buf, 1, ctx)))
		return ret;

	return 0;
}

static int memory(const ax_byte *value, size_t size, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret = sprintf(buf, "%p \\", (void *)value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	size_t shift = 0;
	while (shift != size) {
		size_t partsiz = (size - shift) % (sizeof(buf) / 2 - 1);
		ax_memtoustr(value + shift, partsiz, buf);
		if ((ret = out_cb(buf, partsiz * 2, ctx)))
			return ret;
		shift += partsiz;
	}
	return 0;
}

static int symbol(const char *name, ax_dump_out_cb_f *out_cb, void *ctx)
{
	return out_cb(name, strlen(name), ctx);
}

static int pair_left(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static int pair_midst(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return out_cb(" = ", 3, ctx);
}

static int pair_right(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static int block_left(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
	if (label) {
		if (out_cb(label, strlen(label), ctx))
			return -1;
		if (out_cb(" ", 1, ctx))
			return -1;
	}
	return out_cb("{", 1, ctx);
}

static int block_midst(size_t index, ax_dump_out_cb_f *out_cb, void *ctx)
{
	if (index == 0)
		return 0;
	return out_cb(", ", 2, ctx);
}

static int block_right(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
	return out_cb("}", 1, ctx);
}

static int nomem(ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[] = "!ENOMEM";
	return out_cb(buf, sizeof(buf) - 1, ctx);
}

static int indent(int depth, ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static const struct ax_dump_format_st ax_dump_default_format = 
{
	.snumber = snumber,
	.unumber = unumber,
	.fnumber = fnumber,
	.pointer = pointer,
	.string = string,
	.wstring = wstring,
	.memory = memory,
	.symbol = symbol,
	.pair_left = pair_left,
	.pair_midst = pair_midst,
	.pair_right = pair_right,
	.block_left = block_left,
	.block_midst = block_midst,
	.block_right = block_right,
	.nomem = nomem,
	.indent = indent,
};


const ax_dump_format *ax_dump_get_default_format()
{
	return &ax_dump_default_format;
}

