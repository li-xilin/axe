/*
 * Copyright (c) 2017-2020 Steve Bennett <steveb@workware.net.au>
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to one person obtaining a copy
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

#include "stringbuf.h"
#include "ax/unicode.h"
#include "ax/mem.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <assert.h>

#define SB_INCREMENT 200

stringbuf *sb_alloc(void)
{
	stringbuf *sb = (stringbuf *)malloc(sizeof(*sb));
	sb->remaining = 0;
	sb->last = 0;
	sb->chars = 0;
	sb->data = NULL;

	return(sb);
}

void sb_free(stringbuf *sb)
{
	if (sb) {
		free(sb->data);
	}
	free(sb);
}

static void sb_realloc(stringbuf *sb, int newlen)
{
	sb->data = (ax_uchar *)realloc(sb->data, newlen * sizeof(ax_uchar));
	sb->remaining = newlen - sb->last;
}

void sb_append(stringbuf *sb, const ax_uchar *str)
{
	sb_append_len(sb, str, ax_ustrlen(str));
}

void sb_append_len(stringbuf *sb, const ax_uchar *str, int len)
{
	if (sb->remaining < len + 1) {
		sb_realloc(sb, sb->last + len + 1 + SB_INCREMENT);
	}
	memcpy(sb->data + sb->last, str, len * sizeof(ax_uchar));
	sb->data[sb->last + len] = 0;

	sb->last += len;
	sb->remaining -= len;
#if AX_UCHAR_LEN == 1
	sb->chars += ax_utf8_charcnt(str, len);
#else
	sb->chars += ax_utf16_charcnt(str, len);
#endif
}

ax_uchar *sb_to_string(stringbuf *sb)
{
	if (sb->data == NULL) {
		/* Return an allocated empty string, not null */
		return ax_ustrdup(ax_u(""));
	}
	else {
		/* Just return the data and free the stringbuf structure */
		ax_uchar *pt = sb->data;
		free(sb);
		return pt;
	}
}

/* Insert and delete operations */

/* Moves up all the data at position 'pos' and beyond by 'len' bytes
 * to make room for new data
 *
 * Note: Does *not* update sb->chars
 */
static void sb_insert_space(stringbuf *sb, int pos, int len)
{
	assert(pos <= sb->last);

	/* Make sure there is enough space */
	if (sb->remaining < len) {
		sb_realloc(sb, sb->last + len + SB_INCREMENT);
	}
	/* Now move it up */
	memmove(sb->data + pos + len, sb->data + pos, (sb->last - pos) * sizeof(ax_uchar));
	sb->last += len;
	sb->remaining -= len;
	/* And null terminate */
	sb->data[sb->last] = 0;
}

/**
 * Move down all the data from pos + len, effectively
 * deleting the data at position 'pos' of length 'len'
 */
static void sb_delete_space(stringbuf *sb, int pos, int len)
{
	assert(pos < sb->last);
	assert(pos + len <= sb->last);

#if AX_UCHAR_LEN == 1
	sb->chars -= ax_utf8_charcnt(sb->data + pos, len);
#else
	sb->chars -= ax_utf16_charcnt(sb->data + pos, len);
#endif

	/* Now move it up */
	memmove(sb->data + pos, sb->data + pos + len, (sb->last - pos - len) * sizeof(ax_uchar));
	sb->last -= len;
	sb->remaining += len;
	/* And null terminate */
	sb->data[sb->last] = 0;
}

void sb_insert(stringbuf *sb, int index, const ax_uchar *str)
{
	if (index >= sb->last) {
		/* Inserting after the end of the list appends. */
		sb_append(sb, str);
	}
	else {
		int len = ax_ustrlen(str);

		sb_insert_space(sb, index, len);
		memcpy(sb->data + index, str, len * sizeof(ax_uchar));
		sb->chars += ax_ustr_charcnt(str, len);

	}
}

/**
 * Delete the bytes at index 'index' for length 'len'
 * Has no effect if the index is past the end of the list.
 */
void sb_delete(stringbuf *sb, int index, int len)
{
	if (index < sb->last) {
		ax_uchar *pos = sb->data + index;
		if (len < 0) {
			len = sb->last;
		}

		sb_delete_space(sb, pos - sb->data, len);
	}
}

void sb_clear(stringbuf *sb)
{
	if (sb->data) {
		/* Null terminate */
		sb->data[0] = 0;
		sb->last = 0;
		sb->chars = 0;
	}
}
