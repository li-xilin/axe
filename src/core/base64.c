/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
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

#include "ax/base64.h"
#include "ax/debug.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <errno.h>

static uint8_t alphabet_map[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static uint8_t reverse_map[] = {
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0x3F,
	0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E,
	0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
	0xFF, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
	0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30, 0x31, 0x32, 0x33, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

char* ax_base64_encode(const uint8_t *data, size_t len)
{
	int encode_length = len/3*4;
	if(len % 3 > 0)
		encode_length += 4;

	char *encode = malloc(encode_length + 1);
	if (!encode)
		return NULL;

	size_t i, j;
	for (i = 0, j = 0; i+3 <= len; i += 3) {
		encode[j++] = alphabet_map[data[i] >> 2];
		encode[j++] = alphabet_map[((data[i] << 4)&0x30)|(data[i+1] >> 4)];
		encode[j++] = alphabet_map[((data[i+1] << 2)&0x3c)|(data[i+2] >> 6)];
		encode[j++] = alphabet_map[data[i+2]&0x3f];
	}

	if (i < len) {
		size_t tail = len - i;
		if (tail == 1) {
			encode[j++] = alphabet_map[data[i] >> 2];
			encode[j++] = alphabet_map[(data[i] << 4)&0x30];
			encode[j++] = '=';
			encode[j++] = '=';
		}
		else {
			encode[j++] = alphabet_map[data[i] >> 2];
			encode[j++] = alphabet_map[((data[i] << 4)&0x30)|(data[i+1] >> 4)];
			encode[j++] = alphabet_map[(data[i+1] << 2)&0x3c];
			encode[j++] = '=';
		}
	}
	encode[j] = '\0';
	return encode;
}

uint8_t* ax_base64_decode(const char *base64, size_t len)
{
	if((len & 3) != 0) {
		errno = EINVAL;
		return NULL;
	}

	uint8_t *plain = (uint8_t*)malloc(len/4*3);
	if (!plain)
		return NULL;

	size_t i, j = 0;
	uint8_t quad[4];
	for (i = 0; i < len; i += 4) {
		for (size_t k = 0; k < 4; k++)
			quad[k] = reverse_map[(uint8_t)base64[i+k]];

		if(quad[0] >= 64 || quad[1] >= 64) {
			errno = EILSEQ;
			free(plain);
			return NULL;
		}

		plain[j++] = (quad[0] << 2)|(quad[1] >> 4);

		if (quad[2] >= 64)
			break;
		else if (quad[3] >= 64) {
			plain[j++] = (quad[1] << 4)|(quad[2] >> 2);
			break;
		}
		else {
			plain[j++] = (quad[1] << 4)|(quad[2] >> 2);
			plain[j++] = (quad[2] << 6)|quad[3];
		}
	}
	return plain;
}

