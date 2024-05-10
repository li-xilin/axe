/*
 * Unicode utility functions
 *
 * Copyright (c) 2010-2019 Steve Bennett <steveb@workware.net.au>
 * Copyright (c) 2022-2023 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_UNICODE_H
#define AX_UNICODE_H

#include <stdint.h>
#include <stddef.h>

#define MAX_UTF8_LEN 4

#define AX_UTF8_BOM    (uint8_t[]) { 0xEF, 0xBB, 0xBF }
#define AX_UTF16BE_BOM (uint8_t[]) { 0xFE, 0XFF }
#define AX_UTF16LE_BOM (uint8_t[]) { 0xFF, 0XFE }
#define AX_UTF32BE_BOM (uint8_t[]) { 0x00, 0x00, 0xFE, 0XFF }
#define AX_UTF32LE_BOM (uint8_t[]) { 0xFF, 0XFE, 0x00, 0x00 }

/**
 * Returns the length of the utf-8 sequence starting with 'c'.
 *
 * Returns 1-4, or -1 if this is not a valid start byte.
 *
 * Note that charlen=4 is not supported by the rest of the API.
 */
size_t ax_utf8_charlen(uint8_t c);


/**
 * Returns the number of characters in the utf-8
 * string of the given byte length.
 *
 * Any bytes which are not part of an valid utf-8
 * sequence are treated as individual characters.
 *
 * The string *must* be null terminated.
 *
 * Does not support unicode code points > \u1fffff
 */
size_t ax_utf8_charcnt(const char *str, int bytelen);

/**
 * Calculates the display width of the first 'charlen' characters in 'str'.
 */
size_t ax_utf8_strwidth(const char *str, int charlen);

/**
 * Returns the byte index of the given character in the utf-8 string.
 *
 * The string *must* be null terminated.
 *
 * This will return the byte length of a utf-8 string
 * if given the char length.
 */
size_t ax_utf8_index(const char *str, int charindex);

/**
 * Returns the unicode codepoint corresponding to the
 * utf-8 sequence 'str'.
 *
 * Stores the result in *uc and returns the number of bytes
 * consumed.
 *
 * If 'str' is null terminated, then an invalid utf-8 sequence
 * at the end of the string will be returned as individual bytes.
 *
 * If it is not null terminated, the length *must* be checked first.
 *
 * Does not support unicode code points > \u1fffff
 */
size_t ax_utf8_to_ucode(const char *str, uint32_t *uc);

size_t ax_utf8_to_utf16(const char *utf8, size_t utf8_len, uint16_t* utf16, size_t utf16_len);

/**
 * Returns the width (in characters) of the given unicode codepoint.
 * This is 1 for normal letters and 0 for combining characters and 2 for wide characters.
 */
size_t ax_ucode_width(uint32_t ch);

size_t ax_ucode_to_utf16(uint32_t codepoint, uint16_t* utf16);

size_t ax_ucode_utf8len(uint32_t uc);

/**
 * Converts the given unicode codepoint (0 - 0x1fffff) to utf-8
 * and stores the result at 'p'.
 *
 * Returns the number of utf-8 characters
 */
size_t ax_ucode_to_utf8(uint32_t uc, char *p);

size_t ax_utf16_index(const uint16_t *str, int charindex);

size_t ax_ucode_utf16len(uint32_t codepoint);

size_t ax_utf16_charcnt(const uint16_t* str, int bytelen);

size_t ax_utf16_to_ucode(const uint16_t* utf16, uint32_t *codepoint);

size_t ax_utf16_to_utf8(const uint16_t *utf16, size_t utf16_len, char* utf8, size_t utf8_len);

size_t ax_utf16_strwidth(const uint16_t *str, int charlen);

inline static size_t ax_utf16_strlen(const uint16_t *str)
{
	size_t len = 0;
	while (str[len])
		len++;
	return len;
}

#endif
