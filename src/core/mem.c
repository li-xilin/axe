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

#include "ax/def.h"
#include "ax/mem.h"

#include <stdlib.h>
#include <string.h>
#include <wchar.h>
#include <errno.h>
#include <ctype.h>
#include <stdio.h>

#include "check.h"

#include "strargv.h"
#define WIDE_CHAR
#include "strargv.h"

void ax_memswp(void *p1, void *p2, size_t size)
{
	CHECK_PARAM_NULL(p1);
	CHECK_PARAM_NULL(p2);

	size_t chunk_cnt = size / sizeof(ax_fast_uint),
	       tail_size = size % sizeof(ax_fast_uint);

	ax_fast_uint *pf1 = p1, *pf2 = p2;
	for (size_t i = 0; i < chunk_cnt; i++)
		ax_swap(pf1 + i, pf2 + i, ax_fast_uint);

	uint8_t *ps1 = (ax_byte *)p1 + (size - tail_size),
		*ps2 = (ax_byte *)p2 + (size - tail_size);
	for (size_t i = 0; i != tail_size; i++)
		ax_swap(ps1 + i, ps2 + i, uint8_t);
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

char *ax_strrepl(const char *orig, const char *rep, const char *with)
{
	CHECK_PARAM_NULL(orig);
	CHECK_PARAM_NULL(rep);
	CHECK_PARAM_NULL(with);
	ax_assert(rep[0], "length of parameter rep is 0");

	const char *ins;  /* the next insert point */
	char *result;     /* the return string */
	char *tmp;        /* varies */

	size_t len_rep,   /* length of rep (the string to remove) */
	       len_with,  /* length of with (the string to replace rep with) */
	       len_front, /* distance between rep and end of last rep */
	       count;     /* number of replacements */

	len_rep = strlen(rep);
	if (len_rep == 0)
		return NULL; /* empty rep causes infinite loop during count */
	len_with = strlen(with);

	/* count the number of replacements needed */
	ins = orig;
	for (count = 0; (tmp = strstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

	if (!result)
		return NULL;

	/* first time through the loop, all the variable are set correctly
	 * from here on,
	 *    tmp points to the end of the result string
	 *    ins points to the next occurrence of rep in orig
	 *    orig points to the remainder of orig after "end of rep" */
	while (count--) {
		ins = strstr(orig, rep);
		len_front = ins - orig;
		tmp = strncpy(tmp, orig, len_front) + len_front;
		tmp = strcpy(tmp, with) + len_with;
		orig += len_front + len_rep; // move to next "end of rep"
	}
	strcpy(tmp, orig);
	return result;
}

wchar_t *ax_wcsrepl(const wchar_t *orig, const wchar_t *rep, const wchar_t *with)
{
	CHECK_PARAM_NULL(orig);
	CHECK_PARAM_NULL(rep);
	CHECK_PARAM_NULL(with);
	ax_assert(rep[0], "length of parameter rep is 0");

	const wchar_t *ins;
	wchar_t *result, *tmp;
	size_t len_rep, len_with, len_front, count;

	len_rep = wcslen(rep);
	if (len_rep == 0)
		return NULL;
	len_with = wcslen(with);

	ins = orig;
	for (count = 0; (tmp = wcsstr(ins, rep)); ++count) {
		ins = tmp + len_rep;
	}

	tmp = result = malloc((wcslen(orig) + (len_with - len_rep) * count + 1) * sizeof(wchar_t));
	if (!result)
		return NULL;

	while (count--) {
		ins = wcsstr(orig, rep);
		len_front = ins - orig;
		tmp = wcsncpy(tmp, orig, len_front) + len_front;
		tmp = wcscpy(tmp, with) + len_with;
		orig += len_front + len_rep;
	}
	wcscpy(tmp, orig);
	return result;
}

uint64_t ax_hash64_thomas(uint64_t key)
{
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}

uint64_t ax_hash64_inv_thomas(uint64_t key)
{
	uint64_t tmp;

	// Invert key = key + (key << 31)
	tmp = key-(key<<31);
	key = key-(tmp<<31);

	// Invert key = key ^ (key >> 28)
	tmp = key^key>>28;
	key = key^tmp>>28;

	// Invert key *= 21
	key *= 14933078535860113213u;

	// Invert key = key ^ (key >> 14)
	tmp = key^key>>14;
	tmp = key^tmp>>14;
	tmp = key^tmp>>14;
	key = key^tmp>>14;

	// Invert key *= 265
	key *= 15244667743933553977u;

	// Invert key = key ^ (key >> 24)
	tmp = key^key>>24;
	key = key^tmp>>24;

	// Invert key = (~key) + (key << 21)
	tmp = ~key;
	tmp = ~(key-(tmp<<21));
	tmp = ~(key-(tmp<<21));
	key = ~(key-(tmp<<21));

	return key;
}

inline static void char2hex(char dst[2], ax_byte src)
{
	ax_byte major = (src >> 4) , minor = src & 0x0F;
	dst[0] = (major < 0xA) ? '0' + major : 'A' + (major - 0xA);
	dst[1] = (minor < 0xA) ? '0' + minor : 'A' + (minor - 0xA);
}

char *ax_memtohex(const void *p, size_t size, char *out)
{
	for (int i = 0; i < size; i++)
		char2hex(out + i * 2, ((ax_byte *)p)[i]);
	out[size * 2] = '\0';
	return out;
}

void ax_membyhex(const char *text, void *out)
{
	char *buf = out;
	int i = 0;
	for (i = 0; text[i] != '\0'; i++)
		ax_assert(isdigit((int)text[i]) || (text[i] >= 'A' && text[i] <= 'Z'), "invalid charactor '%c'", text[i]);
	ax_assert(i % 2 == 0, "length of text is odd number");

	for (i = 0; text[i] != '\0'; i += 2)
		buf[i / 2] = (text[i] - (isdigit((int)text[i]) ? '0' : ('A' - 0xA))) * 0x10
			+ (text[i + 1] - (isdigit((int)text[i + 1]) ? '0' : ('A' - 0xA)));
}

size_t ax_strtoargv(char *s, char *argv[], size_t argc_max)
{
	CHECK_PARAM_NULL(s);
	ax_assert(argc_max > 0, "argc_max must be positive integer");
	char *next_ptr = s, *item;
	size_t count = 0;
	while (count < argc_max - 1 && (item = ax_strsplit(&next_ptr, ' '))) {
		if (*item == '\0')
			continue;
		argv[count] = item;
		count++;
	}
	argv[count] = NULL;
	return count;
}

void ax_memxor(void *a, const void *b, size_t size)
{
	int i, j;

	for (i = 0; i < size / sizeof(long); i++)
		((long *)a)[i] ^= ((long *)b)[i];

	for (j = i * sizeof(long); j < size; j++)
		((char *)a)[j] ^= ((char *)b)[j];
}

inline static int char_to_int(char c)
{
        if (c >= '0' && c <= '9')
                return c - '0';
        if (c>= 'a' && c <= 'z')
                return c - 'a' + 0xA;
        if (c>= 'A' && c <= 'Z')
                return c - 'A' + 0xA;
        return -1;
}

inline static char int_to_char(int n)
{
        if (n >= 0 && n < 10)
                return '0' + n;
        if (n >= 10 && n < 36)
                return 'a' + n - 10;
        return '\0';
}

char *ax_strbaseconv(char *s, char *buf, size_t size, int old_base, int new_base)
{
        int j = size - 1;
        ax_assert(old_base >= 2 && old_base <= 36, "Invalid old_base");
        ax_assert(new_base >= 2 && new_base <= 36, "Invalid new_base");
#ifndef NDEBUG
        for (int i = 0; s[i]; i++) {
                int n = char_to_int(s[i]);
                ax_assert(n >= 0 && n < old_base, "Invalid charactor '%c'", s[i]);
        }
#endif
        static const char hex[] = "0123456789abcdefghijklmnopqrstuvwxyz";
        while (1) {
                int i = 0;
                while (s[i] && s[i] == '0')
                        i++;
                if (!s[i])
                        break;
                if (j < 0)
                        return NULL;
                int sum = 0, res = 0;
                for (i = 0; s[i]; i++) {
                        sum = res * old_base + char_to_int(s[i]);
                        s[i] = hex[sum / new_base];
                        res = sum % new_base;
                }
                buf[j--] = int_to_char(sum % new_base);
        }

        return buf + j + 1;
}

char *ax_strtrim(char *s)
{
	int begin = 0, end = -1, i;
	for (i = 0; s[i] && isspace(s[i]); i++);
	begin = i;

	for (; s[i]; i++) {
		if (!isspace(s[i]))
			end = i;
	}

	if (end >= 0)
		s[end + 1] = '\0';
	return s + begin;
}

wchar_t *ax_wcstrim(wchar_t *s)
{
	int begin = 0, end = -1, i;
	for (i = 0; s[i] && isspace(s[i]); i++);
	begin = i;

	for (; s[i]; i++) {
		if (!isspace(s[i]))
			end = i;
	}

	if (end >= 0)
		s[end + 1] = L'\0';
	return s + begin;
}
