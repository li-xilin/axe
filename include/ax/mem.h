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

#ifndef AX_MEM_H
#define AX_MEM_H
#include "def.h"
#include <string.h>

#define ax_swap(_a, _b, _type) \
do { \
	register _type *a = ax_cast(_type *, _a); \
	register _type *b = ax_cast(_type *, _b); \
	register _type tmp = *a; \
	*a = *b; \
	*b = tmp; \
} while(0)

inline static uint32_t ax_hash_murmur32(const void *key, size_t size)
{
	uint32_t h = 3323198485ul;
	for (size_t i = 0; i < size; i++) {
		h ^= ((ax_byte *)key)[i];
		h *= 0x5bd1e995;
		h ^= h >> 15;
	}
	return h;
}

inline static uint64_t ax_hash_murmur64 (const void *key, size_t size)
{
	uint64_t h = 525201411107845655ull;
	for (size_t i = 0; i < size; i++) {
		h ^= ((ax_byte *)key)[i];
		h *= 0x5bd1e9955bd1e995;
		h ^= h >> 47;
	}
	return h;
}

inline static size_t ax_hash_djb(const void *p, size_t size)
{
	size_t h = 5381;
	for (size_t i = 0; i < size; i++) {
		h = (h ^ (h << 5)) ^ ((ax_byte *)p)[i];
	}
	return h;
}

inline static void ax_memswp(void *p1, void *p2, size_t size)
{
	ax_byte tmp[0x1000];
	size_t rest = size;
	while (rest) {
		size_t batch = rest % sizeof tmp;
		memcpy(tmp, p1, batch);
		memcpy(p1, p2, batch);
		memcpy(p2, tmp, batch);
		rest -= batch;
	}
}

void ax_memswp1(void *p1, void *p2, size_t size);

char *ax_strdup(const char *str);

char *ax_strdup2(const char *s, size_t *lenp);

wchar_t *ax_wcsdup(const wchar_t *str);

void *ax_memdup(const void *p, size_t size);

size_t ax_strhash(const char *s);

size_t ax_wcshash(const wchar_t *s);

size_t ax_memhash(const void *p, size_t size);

char *ax_strsplit(char **s, char ch);

char *ax_strrepl(const char *orig, const char *rep, const char *with);

char *ax_memtoustr(const void *p, size_t size, char *buf);

char *ax_memtohex(const void *p, size_t size, char *out);

void ax_membyhex(const char *text, void *out);

#endif

