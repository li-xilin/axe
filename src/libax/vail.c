/*
 * Copyright (c) 2020-2021 Li hsilin <lihsilyn@gmail.com>
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

#include <ax/vail.h>
#include <ax/stuff.h>
#include <ax/debug.h>

#include <stdlib.h>
#include <ctype.h>
#include <assert.h>
#include <string.h>
#include <wchar.h>

#include "check.h"

struct format_st
{
	int8_t size;
	struct 
	{
		uint8_t type;
		uint8_t array;
	} table[0xFF];
};

struct vail_item_st
{
	ax_stuff value;
	size_t size;
	bool is_array;
	uint8_t type;
};

struct ax_vail_st
{
	size_t size;
	struct vail_item_st *table;
};

static int8_t make_format(const char *fmt, struct format_st *vfmt);
static ax_vail* create_vail(struct format_st *vfmt, va_list va);

static ax_vail* create_vail(struct format_st *vfmt, va_list va)
{
	ax_vail* vail = malloc(sizeof(ax_vail));
	if (vail == NULL)
		return NULL;
	vail->table = malloc(sizeof(struct vail_item_st) * (vfmt->size ? vfmt->size : 1));
	if (vail->table == NULL)
		return NULL;

	vail->size = vfmt->size;

	if (vail->size == 0)
		return vail;

	for (int node_i = 0 ; node_i < vfmt->size ; node_i++) {
		struct vail_item_st *item = vail->table + node_i;
		if (vfmt->table[node_i].array) {
			void* arr_ptr = va_arg(va, void*);
			size_t arr_elems = va_arg(va, size_t);
			item->type = vfmt->table[node_i].type;
			item->value.ptr = arr_ptr;
			item->is_array = true;
			item->size = arr_elems;
		} else {
			switch (vfmt->table[node_i].type) {
				case AX_ST_I8:  item->value.i8  = (int8_t)va_arg(va, int32_t); break;
				case AX_ST_I16: item->value.i16 = (int16_t)va_arg(va, int32_t); break;
				case AX_ST_I32: item->value.i32 = va_arg(va, int32_t); break;
				case AX_ST_I64: item->value.i64 = va_arg(va, int64_t); break;
				case AX_ST_U8:  item->value.u8  = (uint8_t)va_arg(va, uint32_t); break;
				case AX_ST_U16: item->value.u16 = (uint16_t)va_arg(va, uint32_t); break;
				case AX_ST_U32: item->value.u32 = va_arg(va, uint32_t); break;
				case AX_ST_U64: item->value.u64 = va_arg(va, uint64_t); break;
				case AX_ST_Z:   item->value.z   = (float)va_arg(va, size_t); break;
				case AX_ST_F:   item->value.f   = (float)va_arg(va, double); break;
				case AX_ST_LF:  item->value.lf  = va_arg(va, double); break;
				case AX_ST_PTR: item->value.ptr = va_arg(va, void*); break;
				case AX_ST_S:   item->value.str = va_arg(va, char*); break;
				case AX_ST_WS:  item->value.wstr = va_arg(va, wchar_t*); break;
			}
			item->type = vfmt->table[node_i].type;
			item->size = 0;
			item->is_array = false;
		}
	}
	return vail;
}

#define TYPE_ACCEPT(__t) { term[term_pos].type = (__t); pos++; break; }

static int8_t make_format(const char *fmt, struct format_st *vfmt)
{
	struct scan_term_st { char type; char mut; };

	if (fmt[0] == '\0') {
		vfmt->size = 0;
		return 0;
	}

	int varg_count = 0, stuff_count = 0;
	int pos = 0;
	char end = 0;
	while (!end) {
		struct scan_term_st term[8]; //TODO
		int term_pos = 0;
		int repeat = 1;
		int term_len = 0;
		for (;;) {
			switch (fmt[pos]) {
				case '&': term[term_pos].mut = 1; pos++; break;
				default: term[term_pos].mut = 0;
			}
			switch (fmt[pos]) {
				case 'i':
				case 'u':
					pos++;
					switch (fmt[pos])
					{
					case '8':
						TYPE_ACCEPT(fmt[pos - 1] == 'i' ? AX_ST_I8 : AX_ST_U8);
					case '1':
						pos++;
						switch (fmt[pos]) {
							case '6':
								TYPE_ACCEPT(fmt[pos - 2] == 'i' ? AX_ST_I16 : AX_ST_U16);
							default:
								goto bad_char;
						}
						break;
					case '3':
						pos++;
						switch (fmt[pos]) {
							case '2':
								TYPE_ACCEPT(fmt[pos - 2] == 'i' ? AX_ST_I32 : AX_ST_U32);
							default:
								goto bad_char;
						}
						break;
					case '6':
						pos++;
						switch (fmt[pos]) {
							case '4':
								TYPE_ACCEPT((fmt[pos - 2] == 'i') ? AX_ST_I64 : AX_ST_U64);
							default:
								goto bad_char;
						}
						break;
					default:
						goto bad_char;
				}
				break;
			case 'f':
				TYPE_ACCEPT(AX_ST_F);
			case 'l':
				pos++;
				switch (fmt[pos]) {
					case 'f':
						TYPE_ACCEPT(AX_ST_LF);
						break;
					default:
						goto bad_char;
				}
				break;
			case 'p':
				TYPE_ACCEPT(AX_ST_PTR);
			case 'z':
				TYPE_ACCEPT(AX_ST_Z);
			case 's':
				if(term[term_pos].mut == 1) {
					goto bad_char;
				}
				TYPE_ACCEPT(AX_ST_S);
			default:
				/*
				if(isupper((int)fmt[pos])) {
					term[term_pos].type = fmt[pos] - 'A' + AX_ST_PWL;
					TYPE_ACCEPT(AX_ST_PTR);
				}
				*/
				if(term_pos == 0 && term[term_pos].mut == 0) {
					goto bad_char;
				}
			}
			switch(fmt[pos]) {
				case '.':
					pos++;
					break;
				default:
					goto end_type_scan;
			}
			term_pos ++;
		} /* end for */

end_type_scan:
		term_len = term_pos + 1;
		switch (fmt[pos]) {
			case 'x':
				pos++;
				if(fmt[pos]>='1' && fmt[pos]<='9') {
					repeat = fmt[pos]-'0';
					pos++;
					while(isdigit((int)fmt[pos])) {
						repeat = repeat * 10 + fmt[pos]-'0';
						if (repeat>20) goto bad_char;
						pos++;
					}
				} else {
					goto bad_char;
				}
		}
		switch (fmt[pos]) {
			case '_':  end = 0; pos++; break;
			case '\0': end = 1; break;
			default:   goto bad_char;
		}

		for(int ri = 0 ; ri < repeat ; ri++) {
			for(int ti = 0 ; ti < term_len ; ti++) {
				char mut = term[ti].mut;
				vfmt->table[stuff_count].array = mut;
				vfmt->table[stuff_count].type = term[ti].type;
				varg_count += (!!mut) + 1;
				if(varg_count == 127 && !end)
					goto too_many_arg;
				stuff_count ++;
			}
		}
	} /* end while */
	vfmt->size = stuff_count;
	return vfmt->size;

too_many_arg:
	ax_assert(false, "too many argument, limited to 127");
	return -1;
bad_char:
#ifdef AX_DEBUG
	{
		char badch[3] = {fmt[pos]};
		if (fmt[pos] == '\0')
			badch[0] = '\\', badch[1] = '0';
		ax_assert(false, "at colume %d, unexpected char '%s' in format string \"%s\"", pos + 1, badch, fmt);
	}
#endif
	return -1;
}

#undef TYPE_ACCEPT

ax_vail *ax_vail_vcreate(const char* fmt, va_list valist)
{
	CHECK_PARAM_NULL(fmt);

	struct format_st argfmt;
	argfmt.size = 0;;
	make_format(fmt, &argfmt);
		
	ax_vail *vail = create_vail(&argfmt, valist);
	return vail;
}

ax_vail *ax_vail_create(const char* fmt, ...)
{
	CHECK_PARAM_NULL(fmt);

	va_list args;
	va_start(args, fmt);
	ax_vail *vail = ax_vail_vcreate(fmt, args);
	va_end(args);
	return vail;
}

void ax_vail_get(ax_vail *vail, uint8_t idx, ax_vail_info *info)
{
	CHECK_PARAM_NULL(vail);
	CHECK_PARAM_NULL(info);

	struct vail_item_st *item = vail->table + idx;
	if (item->is_array) {
		info->ptr = item->value.ptr;
		info->size = item->size;
	} else {
		info->ptr = &item->value;
		info->size = 1;
	}
	info->type = item->type;
}

size_t ax_vail_size(ax_vail *vail)
{
	CHECK_PARAM_NULL(vail);

	return vail->size;
}

void ax_vail_destroy(ax_vail *vail)
{
	free(vail->table);
	free(vail);
}
