#include <ax/def.h>
#include <ax/rb.h>
#include <ax/dump.h>
#include <ax/mem.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>

#define RST  "\x1B[0m"
#define KBLD "\x1B[1m"
#define KRED  "\x1B[31m"
#define KGRN  "\x1B[32m"
#define KYEL  "\x1B[33m"
#define KBLU  "\x1B[34m"
#define KMAG  "\x1B[35m"
#define KCYN  "\x1B[36m"
#define KWHT  "\x1B[37m"

#define FRED(x) KRED x RST
#define FGRN(x) KGRN x RST
#define FYEL(x) KYEL x RST
#define FBLU(x) KBLU x RST
#define FMAG(x) KMAG x RST
#define FCYN(x) KCYN x RST
#define FWHT(x) KWHT x RST

#define BOLD(x) "\x1B[1m" x RST
#define UNDL(x) "\x1B[4m" x RST

static int snumber(int64_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[64];
	int ret = sprintf(buf, FGRN("%" PRIi64), value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int unumber(uint64_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[64];
	int ret = sprintf(buf, FGRN("%" PRIu64), value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int fnumber(double value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[64];
	int ret = sprintf(buf, FGRN("%lg"), value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int pointer(const void *value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[64];
	int ret = sprintf(buf, FGRN("%p"), value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int string(const char *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
{
	int ret;
	if ((ret = out_cb(ax_strcommalen(KMAG "\""), ctx)))
		return ret;
	
	if ((ret = out_cb(value, length, ctx)))
		return ret;

	if ((ret = out_cb(ax_strcommalen("\"" RST), ctx)))
		return ret;

	return 0;
}

static int wstring(const wchar_t *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret;
	if ((ret = out_cb(ax_strcommalen(KMAG "L\""), ctx)))
		return ret;
	mbstate_t mbs = { 0 };
	const wchar_t *p = (void *)value;
	size_t conv = 0;
	buf[sizeof(buf) - 1] = '\0';
	while (p && (conv = wcsrtombs(buf, &p, sizeof(buf) - 1, &mbs)) != 0)
		if ((ret = out_cb(buf, conv, ctx)))
			return ret;
	if (conv == (size_t)-1) 
		if ((ret = out_cb(ax_strcommalen("BADCHAR"), ctx)))
			return ret;
	if ((ret = out_cb(ax_strcommalen("\"" RST), ctx)))
		return ret;

	return 0;
}

static int memory(const ax_byte *value, size_t size, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret;
	if ((ret = out_cb(ax_strcommalen(KBLD KCYN "\\"), ctx)))
		return ret;
	size_t shift = 0;
	while (shift != size) {
		size_t partsiz = (size - shift) % (sizeof(buf) / 2 - 1);
		ax_memtohex(value + shift, partsiz, buf);
		if ((ret = out_cb(buf, partsiz * 2, ctx)))
			return ret;
		shift += partsiz;
	}
	return 0;
}

static int symbol(const char *name, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret = sprintf(buf, BOLD(FRED("%s")), name);
	return out_cb(buf, ret, ctx);
}

static int pair_left(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static int pair_midst(ax_dump_out_cb_f *out_cb, void *ctx)
{
	char equal[] = " " BOLD("=") " ";
	return out_cb(equal, sizeof equal - 1, ctx);
}

static int pair_right(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static int block_left(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
	if (label) {
		if (out_cb(ax_strcommalen(KBLD KRED), ctx))
			return -1;
		if (out_cb(label, strlen(label), ctx))
			return -1;
		if (out_cb(ax_strcommalen(RST RST " "), ctx))
			return -1;
	}
		
	if ( out_cb("{\n", 2, ctx))
		return -1;
	return 0;
}

static int block_midst(size_t index, ax_dump_out_cb_f *out_cb, void *ctx)
{
	if (index == 0)
		return 0;
	return out_cb(",\n", 2, ctx);
}

static int block_right(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
	return out_cb("\n}", 2, ctx);
}

static int nomem(ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[] = "!ENOMEM";
	return out_cb(buf, sizeof(buf) - 1, ctx);
}

static int indent(int depth, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	for (int i = 0; i < depth * 2; i++) {
		buf[i] = ' ';
	}
	return out_cb(buf, depth * 2, ctx);
}

static const struct ax_dump_format_st colored_format = 
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

struct location { float longitude, latitude; };

ax_fail location_init(void *p, va_list *ap)
{
	assert(ap);
	struct location *loc = p;
	loc->longitude = va_arg(*ap, double);
	loc->latitude = va_arg(*ap, double);
	return 0;
}

ax_dump *location_dump(const void *p)
{
	const struct location *loc = p;
	ax_dump *dmp = ax_dump_block("location", 2);
	ax_dump_bind(dmp, 0, ax_dump_pair(ax_dump_symbol("longitude"), ax_dump_float(loc->longitude)));
	ax_dump_bind(dmp, 1, ax_dump_pair(ax_dump_symbol("latitude"), ax_dump_float(loc->latitude)));
	return dmp;
}

void location_free(void *p)
{
	
}

ax_trait_declare(location, struct location,
	INIT(location_init),
	DUMP(location_dump),
	FREE(location_free)
);

int main()
{
	ax_rb_r rb = ax_new(ax_rb, ax_t(str), ax_t(location));
	ax_map_iput(rb.ax_map, "Beijing", 116.405289, 39.904987);
	ax_map_iput(rb.ax_map, "Tianjin", 117.190186, 39.125595);
	ax_map_iput(rb.ax_map, "Suihua", 126.992928, 46.637394);
	ax_map_iput(rb.ax_map, "Dalian", 121.618622, 38.914589);
	ax_map_iput(rb.ax_map, "Xianyabng", 108.705116, 34.333439);
	ax_map_iput(rb.ax_map, "Shanghai", 121.472641, 31.231707);
	ax_dump *dmp = ax_map_dump(rb.ax_map);
	ax_dump_fput(dmp, &colored_format, stdout);
	return 0;
}
