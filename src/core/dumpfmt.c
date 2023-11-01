/*
 * Copyright (c) 2021-2023 Li Xilin <lixilin@gmx.com>
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

#include "ax/dump.h"
#include "ax/mem.h"

#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <wchar.h>
#include <assert.h>

static int default_snumber(intmax_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%" PRIiMAX, value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int default_unumber(uintmax_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%" PRIuMAX, value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int default_fnumber(double value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%lg", value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int default_pointer(const void *value, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[24];
	int ret = sprintf(buf, "%p", value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	return 0;
}

static int default_string(const char *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
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

static int default_wstring(const wchar_t *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret = sprintf(buf, "%p L\"", (void *)value);
	if ((ret = out_cb(buf, ret, ctx)))
		return ret;
	{
		mbstate_t mbs;
		memset(&mbs, 0, sizeof mbs);
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

static int default_memory(const ax_byte *value, size_t size, ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[1024];
	int ret = sprintf(buf, "%p \\", (void *)value);
	if ((ret = out_cb(buf, ret, ctx)))
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

static int default_symbol(const char *name, ax_dump_out_cb_f *out_cb, void *ctx)
{
	return out_cb(name, strlen(name), ctx);
}

static int default_pair_left(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static int default_pair_midst(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return out_cb(" = ", 3, ctx);
}

static int default_pair_right(ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static int default_block_left(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
	if (label) {
		if (out_cb(label, strlen(label), ctx))
			return -1;
		if (out_cb(" ", 1, ctx))
			return -1;
	}
	return out_cb("{", 1, ctx);
}

static int default_block_midst(size_t index, ax_dump_out_cb_f *out_cb, void *ctx)
{
	if (index == 0)
		return 0;
	return out_cb(", ", 2, ctx);
}

static int default_block_right(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
	return out_cb("}", 1, ctx);
}

static int default_nomem(ax_dump_out_cb_f *out_cb, void *ctx)
{
	char buf[] = "!ENOMEM";
	return out_cb(buf, sizeof(buf) - 1, ctx);
}

static int default_indent(int depth, ax_dump_out_cb_f *out_cb, void *ctx)
{
	return 0;
}

static const struct ax_dump_format_st default_format = 
{
	.snumber = default_snumber,
	.unumber = default_unumber,
	.fnumber = default_fnumber,
	.pointer = default_pointer,
	.string = default_string,
	.wstring = default_wstring,
	.memory = default_memory,
	.symbol = default_symbol,
	.pair_left = default_pair_left,
	.pair_midst = default_pair_midst,
	.pair_right = default_pair_right,
	.block_left = default_block_left,
	.block_midst = default_block_midst,
	.block_right = default_block_right,
	.nomem = default_nomem,
	.indent = default_indent,
};

static int pretty_snumber(int64_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[64];
        int ret = sprintf(buf, "%" PRIi64, value);
        if ((ret = out_cb(buf, ret, ctx)))
                return ret;
        return 0;
}

static int pretty_unumber(uint64_t value, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[64];
        int ret = sprintf(buf, "%" PRIu64, value);
        if ((ret = out_cb(buf, ret, ctx)))
                return ret;
        return 0;
}

static int pretty_fnumber(double value, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[64];
        int ret = sprintf(buf, "%lg", value);
        if ((ret = out_cb(buf, ret, ctx)))
                return ret;
        return 0;
}

static int pretty_pointer(const void *value, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[64];
        int ret = sprintf(buf, "%p", value);
        if ((ret = out_cb(buf, ret, ctx)))
                return ret;
        return 0;
}

static int pretty_string(const char *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
{
        int ret;
        if ((ret = out_cb(ax_strcommalen("\""), ctx)))
                return ret;

        if ((ret = out_cb(value, length, ctx)))
                return ret;

        if ((ret = out_cb(ax_strcommalen("\""), ctx)))
                return ret;

        return 0;
}

static int pretty_wstring(const wchar_t *value, size_t length, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[1024];
        int ret;
        if ((ret = out_cb(ax_strcommalen("L\""), ctx)))
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
        if ((ret = out_cb(ax_strcommalen("\""), ctx)))
                return ret;

        return 0;
}

static int pretty_memory(const ax_byte *value, size_t size, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[1024];
        int ret;
        if ((ret = out_cb(ax_strcommalen("\\"), ctx)))
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

static int pretty_symbol(const char *name, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[1024];
        int ret = sprintf(buf, "%s", name);
        return out_cb(buf, ret, ctx);
}

static int pretty_pair_left(ax_dump_out_cb_f *out_cb, void *ctx)
{
        return 0;
}

static int pretty_pair_midst(ax_dump_out_cb_f *out_cb, void *ctx)
{
        char equal[] = " " "=" " ";
        return out_cb(equal, sizeof equal - 1, ctx);
}

static int pretty_pair_right(ax_dump_out_cb_f *out_cb, void *ctx)
{
        return 0;
}

static int pretty_block_left(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
        if (label) {
                if (out_cb(label, strlen(label), ctx))
                        return -1;
	}

        if (out_cb(" {\n", 3, ctx))
                return -1;
        return 0;
}

static int pretty_block_midst(size_t index, ax_dump_out_cb_f *out_cb, void *ctx)
{
        if (index == 0)
                return 0;
        return out_cb(",\n", 2, ctx);
}

static int pretty_block_right(const char *label, ax_dump_out_cb_f *out_cb, void *ctx)
{
        return out_cb("\n}", 2, ctx);
}

static int pretty_nomem(ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[] = "!ENOMEM";
        return out_cb(buf, sizeof(buf) - 1, ctx);
}

static int pretty_indent(int depth, ax_dump_out_cb_f *out_cb, void *ctx)
{
        char buf[1024];
        for (int i = 0; i < depth * 2; i++) {
                buf[i] = ' ';
        }
        return out_cb(buf, depth * 2, ctx);
}

static const struct ax_dump_format_st pretty_format =
{
        .snumber = pretty_snumber,
        .unumber = pretty_unumber,
        .fnumber = pretty_fnumber,
        .pointer = pretty_pointer,
        .string = pretty_string,
        .wstring = pretty_wstring,
        .memory = pretty_memory,
        .symbol = pretty_symbol,
        .pair_left = pretty_pair_left,
        .pair_midst = pretty_pair_midst,
        .pair_right = pretty_pair_right,
        .block_left = pretty_block_left,
        .block_midst = pretty_block_midst,
        .block_right = pretty_block_right,
        .nomem = pretty_nomem,
        .indent = pretty_indent,
};

const ax_dump_format *ax_dump_default_format()
{
	return &default_format;
}

const ax_dump_format *ax_dump_pretty_format()
{
	return &pretty_format;
}

