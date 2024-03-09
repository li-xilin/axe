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

#ifndef AX_MEM_H
#define AX_MEM_H
#include "def.h"
#include <string.h>

#define ax_swap(a, b, type) do { type tmp = *(a); *(a) = *(b); *(b) = tmp; } while(0)

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

void ax_memswp(void *p1, void *p2, size_t size);

char *ax_strdup(const char *str);

char *ax_strdup2(const char *s, size_t *lenp);

wchar_t *ax_wcsdup(const wchar_t *str);

void *ax_memdup(const void *p, size_t size);

size_t ax_strhash(const char *s);

size_t ax_wcshash(const wchar_t *s);

size_t ax_memhash(const void *p, size_t size);

uint64_t ax_hash64_thomas(uint64_t key);

uint64_t ax_hash64inv_thomas(uint64_t key);

char *ax_strsplit(char **s, char ch);

size_t ax_strtoargv(char *s, char *argv[], size_t len);

char *ax_strrepl(const char *orig, const char *rep, const char *with);

wchar_t *ax_wcsrepl(const wchar_t *orig, const wchar_t *rep, const wchar_t *with);

char *ax_memtoustr(const void *p, size_t size, char *buf);

char *ax_memtohex(const void *p, size_t size, char *out);

void ax_membyhex(const char *text, void *out);

char **ax_strargv(const char *cmdline, int* count);

wchar_t **ax_wcsargv(const wchar_t *cmdline, int* count);

void ax_memxor(void *a, const void *b, size_t size);

char *ax_strbaseconv(char *s, char *buf, size_t size, int old_base, int new_base);

char *ax_strtrim(char *s);

wchar_t *ax_wcstrim(wchar_t *s);

#endif

