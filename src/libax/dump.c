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

#include <ax/dump.h>
#include <ax/mem.h>

#include "check.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>
#include <stdbool.h>
#include <inttypes.h>
#include <ctype.h>

#ifdef AX_DEBUG
#define check_symbol(_sym) \
do { \
	CHECK_PARAM_NULL(_sym); \
	ax_assert(_sym[0] != '\0', \
			"invalid zero-length symbol name"); \
	ax_assert(isalpha(_sym[0]) || _sym[0] == '_', \
			"symbol name %s begin with invalid charactor", _sym); \
	for (int i = 1; _sym[i]; i++) { \
		ax_assert(_sym[i] == '_' || _sym[i] == '.' || isdigit(_sym[i]) || isalpha(_sym[i]), \
				"invalid charactor \'%c\' in symbol name \'%s\'", _sym[i], _sym); \
	} \
} while(0)
#else
#define check_symbol(_sym) (void)0
#endif

enum {
	DTYPE_SNUM,
	DTYPE_UNUM,
	DTYPE_FNUM,
	DTYPE_PTR,
	DTYPE_STR,
	DTYPE_WCS,
	DTYPE_MEM,
	DTYPE_SYM,
	DTYPE_PAIR,
	DTYPE_BLOCK,
};

#define DTYPE_BIND 0x0100

struct search_args
{
       	char *buf;
	size_t size;
	ax_dump_out_cb *cb;
	void *ctx;
};

union value_u
{
	int64_t snum;
	uint64_t unum;
	double fnum;
	const void *ptr;

	struct value_mem_st {
		const void *maddr;
		size_t size;
		char data[];
	} mem, str, wcs, sym;

	struct value_pair_st {
		ax_dump *first;
		ax_dump *second;
	} pair;

	struct value_block_st {
		size_t len;
		char *name;
		ax_dump *dumps[];
	} block;
};

struct ax_dump_st
{
	int type;
	char value[];
};

ax_dump *ax_dump_int(int64_t val)
{
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof val);
	if (!dmp)
		return NULL;
	dmp->type = DTYPE_SNUM;
	((union value_u *)dmp->value)->snum = val;
	return dmp;
}

ax_dump *ax_dump_uint(uint64_t val)
{
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof val);
	if (!dmp)
		return NULL;
	dmp->type = DTYPE_UNUM;
	((union value_u *)dmp->value)->unum = val;
	return dmp;
}

ax_dump *ax_dump_float(double val)
{
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof val);
	if (!dmp)
		return NULL;
	dmp->type = DTYPE_FNUM;
	((union value_u *)dmp->value)->fnum = val;
	return dmp;
}

ax_dump *ax_dump_ptr(const void *val)
{
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof val);
	if (!dmp)
		return NULL;
	dmp->type = DTYPE_PTR;
	((union value_u *)dmp->value)->ptr = val;
	return dmp;

}

ax_dump *ax_dump_str(const char *val)
{
	if (val == NULL)
		return ax_dump_symbol("NULL");
	size_t len = strlen(val);
	size_t siz = (len + 1) * sizeof(char);
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof(struct value_mem_st) + siz);
	if (!dmp)
		return NULL;

	dmp->type = DTYPE_STR;
	union value_u *value = (void *)dmp->value;
	value->str.size = len;
	value->str.maddr = val;
	memcpy(value->str.data, val, siz);

	return dmp;
}

ax_dump *ax_dump_wcs(const wchar_t *val)
{
	if (val == NULL)
		return ax_dump_symbol("NULL");
	size_t len = wcslen(val);
	size_t siz = (len + 1) * sizeof(wchar_t);
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof(struct value_mem_st) + siz);
	if (!dmp)
		return NULL;

	dmp->type = DTYPE_WCS;
	union value_u *value = (void *)dmp->value;
	value->wcs.size = len;
	value->str.maddr = val;
	memcpy(value->wcs.data, val, siz);

	return dmp;
}

ax_dump *ax_dump_mem(const void *ptr, size_t size)
{
	if (ptr == NULL)
		return ax_dump_symbol("NULL");

	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof(struct value_mem_st) + size);
	if (!dmp)
		return NULL;

	dmp->type = DTYPE_MEM;
	union value_u *value = (void *)dmp->value;
	value->mem.size = size;
	value->str.maddr = ptr;
	memcpy(value->mem.data, ptr, size);

	return dmp;
}


ax_dump *ax_dump_symbol(const char *sym)
{
	check_symbol(sym);

	size_t len = strlen(sym);
	size_t siz = (len + 1) * sizeof(char);
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof(struct value_mem_st) + siz);
	if (!dmp)
		return NULL;

	dmp->type = DTYPE_SYM;
	union value_u *value = (void *)dmp->value;
	value->sym.size = len;
	memcpy(value->sym.data, sym, siz);

	return dmp;
}

ax_dump *ax_dump_pair()
{
	ax_dump *dmp = malloc(sizeof(ax_dump) + sizeof(struct value_pair_st));
	if (!dmp)
		return NULL;

	dmp->type = DTYPE_PAIR;
	struct value_pair_st *pair = (void *)dmp->value;
	pair->first = NULL;
	pair->second = NULL;

	return dmp;
}

ax_dump *ax_dump_block(const char *sym, size_t len)
{
	check_symbol(sym);

	ax_dump *dmp = calloc(1, sizeof(ax_dump) + sizeof(struct value_block_st) + len * sizeof (ax_dump *) );
	if (!dmp)
		return NULL;

	dmp->type = DTYPE_BLOCK;
	struct value_block_st *block = (void *) dmp->value;
	block->len = len;
	block->name = ax_strdup(sym);
	if (!block->name ) {
		free(dmp);
		return NULL;
	}

	for (size_t i = 0; i < len; i++) {
		block->dumps[i] = NULL;
	}

	return dmp;
}

ax_fail ax_dump_set_name(ax_dump *dmp, const char *sym)
{
	check_symbol(sym);
	ax_assert(dmp->type == DTYPE_BLOCK, "unsupported dump type");
	struct value_block_st *block = (void *) dmp->value;
	free(block->name);
	block->name = ax_strdup(sym);
	if (!block->name ) {
		free(dmp);
		return true;
	}
	return false;
}

void ax_dump_bind(ax_dump *dmp, int index, ax_dump* binding)
{
	CHECK_PARAM_NULL(dmp);
	CHECK_PARAM_NULL(index >= 0);
	CHECK_PARAM_NULL(binding);
	ax_assert(dmp->type == DTYPE_PAIR || dmp->type == DTYPE_BLOCK,
			"binding operation only support pair and block"); 
	ax_assert(!(binding->type & DTYPE_BIND),
			"instance is already in binding");

	union {
		struct value_block_st *block;
		struct value_pair_st *pair;
	} val;

#ifdef AX_DEBUG
	switch (binding->type) {
		case DTYPE_PAIR:
			val.pair = (void *) binding->value;
			ax_assert(val.pair->first, "incomplete dump of pair, NULL pointer field of first");
			ax_assert(val.pair->second, "incomplete dump of pair, NULL pointer field of second");
			break;
		case DTYPE_BLOCK:
			val.block = (void *) binding->value;
			for (int i = 0; i < val.block->len; i++) {
				ax_assert(val.block->dumps[i], "incomplete dump of block, NULL pointer field #%d", i);
			}
			break;
	}
#endif

	switch (dmp->type) {
		case DTYPE_PAIR:
			val.pair = (void *) dmp->value;
			ax_assert(index == 0 || index == 1, "exceed index %d", index);
			switch (index) {
				case 0:
					ax_assert(!val.pair->first, "first field exists value");
					val.pair->first = binding;
					binding->type |= DTYPE_BIND;
					break;
				case 1:
					ax_assert(!val.pair->second, "second field exists value");
					val.pair->second = binding;
					binding->type |= DTYPE_BIND;
					break;
			}
			break;
		case DTYPE_BLOCK:
			val.block = (void *) dmp->value;
			ax_assert(index >= 0 && index < val.block->len, "dump: exceed block field index %d", index);
			ax_assert(!val.block->dumps[index], "field #%d already set", index);
			val.block->dumps[index] = binding;
			binding->type |= DTYPE_BIND;
			break;
	}
}

static void dump_rec_free(ax_dump *dmp)
{
	if (!dmp)
		return;
	assert((dmp->type & DTYPE_BIND));
	union {
		struct value_block_st *block;
		struct value_pair_st *pair;
	} val;
	switch (dmp->type & ~DTYPE_BIND) {
		case DTYPE_SNUM:
		case DTYPE_UNUM:
		case DTYPE_FNUM:
		case DTYPE_PTR:
		case DTYPE_STR:
		case DTYPE_WCS:
		case DTYPE_MEM:
		case DTYPE_SYM:
			break;
		case DTYPE_PAIR:
			val.pair = (void *) dmp->value;
			dump_rec_free(val.pair->first);
			dump_rec_free(val.pair->second);
			break;
		case DTYPE_BLOCK:
			val.block = (void *) dmp->value;
			for (size_t i = 0; i < val.block->len; i++) {
				dump_rec_free(val.block->dumps[i]);
			}
			free(val.block->name);
			break;
		default:
			abort();
	}
	free(dmp);
}

void ax_dump_free(ax_dump *dmp)
{
	CHECK_PARAM_NULL(dmp);
	ax_assert(!(dmp->type & DTYPE_BIND),
			"failed to free dump, the instance has beed bound");

	dmp->type |= DTYPE_BIND;
	dump_rec_free(dmp);
}

int write_file_cb(const char *buf, size_t len, void *ctx)
{
	FILE *fp = ctx;
	if (fwrite(buf, 1, len, fp) != len)
		return -1;
	return 0;
}

ax_fail ax_dump_fput(const ax_dump *dmp, FILE *fp)
{
	return ax_dump_out(dmp, write_file_cb, fp) ? true : false;
}

static void char2hex(char dst[2], char src)
{
	char major = src >> 4,
	     minor = src & 0x0F;
	
	dst[0] = major < 0xA ? '0' + major : 'A' + major;
	dst[1] = minor < 0xA ? '0' + minor : 'A' + major;
}

int dump_out_dfs(const ax_dump *dmp, int depth, struct search_args *args)
{
	int rc;
	union value_u *value = (void *)dmp->value;
	switch (dmp->type & ~DTYPE_BIND) {
		case DTYPE_SNUM:
			rc = sprintf(args->buf, "%" PRIi64, value->snum);
			if ((rc = args->cb(args->buf, rc, args->ctx)))
				return rc;
			break;
		case DTYPE_UNUM:
			rc = sprintf(args->buf, "%" PRIu64, value->unum);
			if ((rc = args->cb(args->buf, rc, args->ctx)))
				return rc;
			break;
		case DTYPE_FNUM:
			rc = sprintf(args->buf, "%lg", value->fnum);
			if ((rc = args->cb(args->buf, rc, args->ctx)))
				return rc;
			break;
		case DTYPE_PTR:
			rc = sprintf(args->buf, "%p", value->ptr);
			if ((rc = args->cb(args->buf, rc, args->ctx)))
				return rc;
			break;
		case DTYPE_STR:
			rc = sprintf(args->buf, "%p \"", value->str.maddr);
			if ((rc = args->cb(args->buf, rc, args->ctx)))
				return rc;
			if ((rc = args->cb(value->str.data, value->str.size, args->ctx)))
				return rc;
			strcpy(args->buf, "\"");
			if ((rc = args->cb(args->buf, 1, args->ctx)))
				return rc;
			break;
		case DTYPE_WCS:

			rc = sprintf(args->buf, "%p L\"", value->str.maddr);
			if ((rc = args->cb(args->buf, rc, args->ctx)))
				return rc;
			{
				mbstate_t mbs = { 0 };
				const wchar_t *p = (void *)value->wcs.data;
				size_t conv = 0;
				args->buf[args->size - 1] = '\0';
				while (p && (conv = wcsrtombs(args->buf, &p, args->size - 1, &mbs)) != 0)
					if ((rc = args->cb(args->buf, conv, args->ctx)))
						return rc;
				if (conv == (size_t)-1) 
					if ((rc = args->cb("BADCHAR", sizeof "BADCHAR" -1, args->ctx)))
						return rc;
			}
			strcpy(args->buf, "\"");
			if ((rc = args->cb(args->buf, 1, args->ctx)))
				return rc;

			break;
		case DTYPE_MEM:
			rc = sprintf(args->buf, "%p \\", value->str.maddr);
			if ((rc = args->cb(args->buf, rc, args->ctx)))
				return rc;
			{
				assert(args->size >= 3);
				size_t part_siz = (args->size - 1) / 2;
				for (size_t j = 0; j < value->mem.size / part_siz; j++) {
					for (size_t i = 0; i < part_siz; i++)
						char2hex(args->buf + i * 2, value->mem.data[j * part_siz + i]);
					args->buf[part_siz * 2] = '\0';
					if ((rc = args->cb(args->buf, part_siz, args->ctx)))
						return rc;
				}
				size_t rest_siz = value->mem.size % part_siz;
				for (size_t i = 0; i < rest_siz; i++)
					char2hex(args->buf + i * 2, value->mem.data[value->mem.size - rest_siz + i]);
				args->buf[rest_siz * 2] = '\0';
				if ((rc = args->cb(args->buf, rest_siz * 2, args->ctx)))
					return rc;

			}
			break;
		case DTYPE_SYM:
			if ((rc = args->cb(value->sym.data, value->sym.size, args->ctx)))
				return rc;
			break;
		case DTYPE_PAIR:
			if ((rc = dump_out_dfs(value->pair.first, depth + 1, args)))
				return rc;
			strcpy(args->buf, " = ");
			args->cb(args->buf, 3, args->ctx);
			if ((rc = dump_out_dfs(value->pair.second, depth + 1, args)))
				return rc;
			break;
		case DTYPE_BLOCK:
			if ((rc = args->cb(value->block.name, strlen(value->block.name), args->ctx)))
				return rc;
			strcpy(args->buf, " {");
			if ((rc = args->cb(args->buf, 2, args->ctx)))
				return rc;

			for (size_t i = 0; i < value->block.len; i++) {
				if (i != 0) {
					strcpy(args->buf, ", ");
					if ((rc = args->cb(args->buf, 2, args->ctx)))
						return rc;
				}

				if ((rc = dump_out_dfs(value->block.dumps[i], depth, args)))
					return rc;
			}
			strcpy(args->buf, "}");
			if ((rc = args->cb(args->buf, 2, args->ctx)))
				return rc;
			break;

		default:
			abort();
	}
	return 0;
}

int ax_dump_out(const ax_dump *dmp, ax_dump_out_cb *cb, void *ctx)
{
	CHECK_PARAM_NULL(dmp);
	CHECK_PARAM_NULL(cb);

	char buf[1024];
	struct search_args args = {
		.buf = buf,
		.size = sizeof buf,
		.cb = cb,
		.ctx = ctx
	};
	return dump_out_dfs(dmp, 0, &args);
}

