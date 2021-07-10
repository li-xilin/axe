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

#include <ax/def.h>
#include <ax/mem.h>

#include <stdlib.h>
#include <string.h>
#include <wchar.h>

#include "check.h"
void ax_memxor(void *ptr1, void *ptr2, size_t size)
{
	CHECK_PARAM_NULL(ptr1);
	CHECK_PARAM_NULL(ptr2);

	if (ptr1 == ptr2)
		return;

	register size_t fast_size = size / sizeof(ax_fast_uint);
	register size_t slow_size = size % sizeof(ax_fast_uint);

	ax_fast_uint *pf1 = ptr1, *pf2 = ptr2;
	for (size_t i = 0; i!= fast_size; i++) {
		pf1[i] = pf1[i] ^ pf2[i];
		pf2[i] = pf1[i] ^ pf2[i];
		pf1[i] = pf1[i] ^ pf2[i];
	}

	uint8_t *p1 = (ax_byte *)ptr1 + size - slow_size, *p2 = (ax_byte*)ptr2 + size - slow_size;
	for (size_t i = 0; i!= slow_size; i++) {
		p1[i] = p1[i] ^ p2[i];
		p2[i] = p1[i] ^ p2[i];
		p1[i] = p1[i] ^ p2[i];
	}
}

char *ax_strdup(const char *s)
{
	CHECK_PARAM_NULL(s);

	size_t size = (strlen(s) + 1) * sizeof(char);
	char *copy = malloc(size);
	if (!copy)
		return NULL;
	memcpy(copy, s, size);
	return copy;
}

char *ax_strdup2(const char *s, size_t *lenp)
{
	CHECK_PARAM_NULL(s);

	size_t len = strlen(s);
	size_t size = (len + 1) * sizeof(char);
	char *copy = malloc(size);
	if (!copy)
		return NULL;
	memcpy(copy, s, size);
	*lenp = len;
	return copy;
}


wchar_t *ax_wcsdup(const wchar_t* s)
{
	CHECK_PARAM_NULL(s);

	size_t size = (wcslen(s) + 1) * sizeof(wchar_t);
	wchar_t *copy = malloc(size);
	if (!copy)
		return NULL;
	memcpy(copy, s, size);
	return copy;
}

void *ax_memdup(const void *p, size_t size)
{
	CHECK_PARAM_NULL(p);

	ax_byte *copy = malloc(size);
	if (!copy)
		return NULL;
	memcpy(copy, p, size);
	return copy;
}

size_t ax_strhash(const char *s)
{
	CHECK_PARAM_NULL(s);

	register size_t h = 5381;
	int c;
	while ((c = *s++)) {
		h = (h ^ (h << 5)) ^ c; /* hash * 33 + c */
	}
	return h;
}

size_t ax_wcshash(const wchar_t *s)
{
	CHECK_PARAM_NULL(s);

	register size_t h = 5381;
	int c;
	while ((c = *s++)) {
		h = (h ^ (h << 5)) ^ c; /* hash * 33 + c */
	}
	return h;
}

size_t ax_memhash(const void *p, size_t size)
{
	CHECK_PARAM_NULL(p);

	register size_t h = 5381;
	for (size_t i = 0; i < size; i++) {
		h = (h ^ (h << 5)) ^ ((ax_byte *)p)[i];
	}
	return h;
}

char *ax_strsplit(char **s, char ch)
{
	CHECK_PARAM_NULL(s);

	char *ret;
	if (*s) {
		for (char *p = *s; *p; p++) {
			if (*p != ch)
				continue;
			*p = '\0';
			ret = *s;
			*s = p + 1;
			return ret ;
		}
		ret = *s;
	} else {
		ret = NULL;
	}
	*s = NULL;
	return ret;
}
