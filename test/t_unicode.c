/*
 * Copyright (c) 2023 Li hsilin <lihsilyn@gmail.com>
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

#include "ax/unicode.h"
#include "ut/runner.h"
#include "ut/suite.h"

#include <assert.h>
#include <setjmp.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

uint8_t utf8_text[] = {
	0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1, 0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6,
	0xe5, 0x85, 0xb3, 0xe4, 0xb9, 0x8e, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1,
	0xef, 0xbc, 0x8c, 0xe8, 0x80, 0x8c, 0xe9, 0x9d, 0x9e, 0xe4, 0xbb, 0xb7,
	0xe6, 0xa0, 0xbc, 0xe3, 0x80, 0x82, 0xe8, 0xa6, 0x81, 0xe7, 0x90, 0x86,
	0xe8, 0xa7, 0xa3, 0xe8, 0xbf, 0x99, 0xe4, 0xb8, 0xaa, 0xe6, 0xa6, 0x82,
	0xe5, 0xbf, 0xb5, 0xef, 0xbc, 0x8c, 0xe4, 0xbd, 0xa0, 0xe5, 0xba, 0x94,
	0xe8, 0xaf, 0xa5, 0xe8, 0x80, 0x83, 0xe8, 0x99, 0x91, 0x20, 0xe2, 0x80,
	0x9c, 0x66, 0x72, 0x65, 0x65, 0xe2, 0x80, 0x9d, 0x20, 0xe6, 0x98, 0xaf,
	0x20, 0xe2, 0x80, 0x9c, 0xe8, 0xa8, 0x80, 0xe8, 0xae, 0xba, 0xe8, 0x87,
	0xaa, 0xe7, 0x94, 0xb1, 0xef, 0xbc, 0x88, 0x66, 0x72, 0x65, 0x65, 0x20,
	0x73, 0x70, 0x65, 0x65, 0x63, 0x68, 0xef, 0xbc, 0x89, 0xe2, 0x80, 0x9d,
	0xe4, 0xb8, 0xad, 0xe7, 0x9a, 0x84, 0xe2, 0x80, 0x9c, 0xe8, 0x87, 0xaa,
	0xe7, 0x94, 0xb1, 0xe2, 0x80, 0x9d, 0xef, 0xbc, 0x9b, 0xe8, 0x80, 0x8c,
	0xe4, 0xb8, 0x8d, 0xe6, 0x98, 0xaf, 0x20, 0xe2, 0x80, 0x9c, 0xe5, 0x85,
	0x8d, 0xe8, 0xb4, 0xb9, 0xe5, 0x95, 0xa4, 0xe9, 0x85, 0x92, 0xef, 0xbc,
	0x88, 0x66, 0x72, 0x65, 0x65, 0x20, 0x62, 0x65, 0x65, 0x72, 0xef, 0xbc,
	0x89, 0xe2, 0x80, 0x9d, 0xe4, 0xb8, 0xad, 0xe7, 0x9a, 0x84, 0xe2, 0x80,
	0x9c, 0xe5, 0x85, 0x8d, 0xe8, 0xb4, 0xb9, 0xe2, 0x80, 0x9d, 0xe3, 0x80,
	0x82, 0x0a, 0x0a, 0xe6, 0x9b, 0xb4, 0xe7, 0xb2, 0xbe, 0xe7, 0xa1, 0xae,
	0xe5, 0x9c, 0xb0, 0xe8, 0xaf, 0xb4, 0xef, 0xbc, 0x8c, 0xe8, 0x87, 0xaa,
	0xe7, 0x94, 0xb1, 0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe8, 0xb5, 0x8b,
	0xe4, 0xba, 0x88, 0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe4, 0xbd, 0xbf,
	0xe7, 0x94, 0xa8, 0xe8, 0x80, 0x85, 0xe5, 0x9b, 0x9b, 0xe9, 0xa1, 0xb9,
	0xe5, 0x9f, 0xba, 0xe6, 0x9c, 0xac, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1,
	0xef, 0xbc, 0x9a, 0x0a, 0x0a, 0xe4, 0xb8, 0x8d, 0xe8, 0xae, 0xba, 0xe7,
	0x9b, 0xae, 0xe7, 0x9a, 0x84, 0xe4, 0xb8, 0xba, 0xe4, 0xbd, 0x95, 0xef,
	0xbc, 0x8c, 0xe6, 0x9c, 0x89, 0xe8, 0xbf, 0x90, 0xe8, 0xa1, 0x8c, 0xe8,
	0xaf, 0xa5, 0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe7, 0x9a, 0x84, 0xe8,
	0x87, 0xaa, 0xe7, 0x94, 0xb1, 0xef, 0xbc, 0x88, 0xe8, 0x87, 0xaa, 0xe7,
	0x94, 0xb1, 0xe4, 0xb9, 0x8b, 0xe9, 0x9b, 0xb6, 0xef, 0xbc, 0x89, 0xe3,
	0x80, 0x82, 0x0a, 0xe6, 0x9c, 0x89, 0xe7, 0xa0, 0x94, 0xe7, 0xa9, 0xb6,
	0xe8, 0xaf, 0xa5, 0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe5, 0xa6, 0x82,
	0xe4, 0xbd, 0x95, 0xe5, 0xb7, 0xa5, 0xe4, 0xbd, 0x9c, 0xe4, 0xbb, 0xa5,
	0xe5, 0x8f, 0x8a, 0xe6, 0x8c, 0x89, 0xe9, 0x9c, 0x80, 0xe6, 0x94, 0xb9,
	0xe5, 0x86, 0x99, 0xe8, 0xaf, 0xa5, 0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6,
	0xe7, 0x9a, 0x84, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1, 0xef, 0xbc, 0x88,
	0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1, 0xe4, 0xb9, 0x8b, 0xe4, 0xb8, 0x80,
	0xef, 0xbc, 0x89, 0xe3, 0x80, 0x82, 0xe5, 0x8f, 0x96, 0xe5, 0xbe, 0x97,
	0xe8, 0xaf, 0xa5, 0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe6, 0xba, 0x90,
	0xe4, 0xbb, 0xa3, 0xe7, 0xa0, 0x81, 0xe4, 0xb8, 0xba, 0xe8, 0xbe, 0xbe,
	0xe6, 0x88, 0x90, 0xe6, 0xad, 0xa4, 0xe7, 0x9b, 0xae, 0xe7, 0x9a, 0x84,
	0xe4, 0xb9, 0x8b, 0xe5, 0x89, 0x8d, 0xe6, 0x8f, 0x90, 0xe3, 0x80, 0x82,
	0x0a, 0xe6, 0x9c, 0x89, 0xe9, 0x87, 0x8d, 0xe6, 0x96, 0xb0, 0xe5, 0x8f,
	0x91, 0xe5, 0xb8, 0x83, 0xe6, 0x8b, 0xb7, 0xe8, 0xb4, 0x9d, 0xe7, 0x9a,
	0x84, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1, 0xef, 0xbc, 0x8c, 0xe8, 0xbf,
	0x99, 0xe6, 0xa0, 0xb7, 0xe4, 0xbd, 0xa0, 0xe5, 0x8f, 0xaf, 0xe4, 0xbb,
	0xa5, 0xe5, 0x80, 0x9f, 0xe6, 0xad, 0xa4, 0xe6, 0x9d, 0xa5, 0xe6, 0x95,
	0xa6, 0xe4, 0xba, 0xb2, 0xe7, 0x9d, 0xa6, 0xe9, 0x82, 0xbb, 0xef, 0xbc,
	0x88, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1, 0xe4, 0xb9, 0x8b, 0xe4, 0xba,
	0x8c, 0xef, 0xbc, 0x89, 0xe3, 0x80, 0x82, 0x0a, 0xe6, 0x9c, 0x89, 0xe5,
	0x90, 0x91, 0xe5, 0x85, 0xac, 0xe4, 0xbc, 0x97, 0xe5, 0x8f, 0x91, 0xe5,
	0xb8, 0x83, 0xe6, 0x94, 0xb9, 0xe8, 0xbf, 0x9b, 0xe7, 0x89, 0x88, 0xe8,
	0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe7, 0x9a, 0x84, 0xe8, 0x87, 0xaa, 0xe7,
	0x94, 0xb1, 0xef, 0xbc, 0x88, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1, 0xe4,
	0xb9, 0x8b, 0xe4, 0xb8, 0x89, 0xef, 0xbc, 0x89, 0xef, 0xbc, 0x8c, 0xe8,
	0xbf, 0x99, 0xe6, 0xa0, 0xb7, 0xe6, 0x95, 0xb4, 0xe4, 0xb8, 0xaa, 0xe7,
	0xa4, 0xbe, 0xe7, 0xbe, 0xa4, 0xe9, 0x83, 0xbd, 0xe5, 0x8f, 0xaf, 0xe5,
	0x9b, 0xa0, 0xe6, 0xad, 0xa4, 0xe5, 0x8f, 0x97, 0xe6, 0x83, 0xa0, 0xe3,
	0x80, 0x82, 0xe5, 0x8f, 0x96, 0xe5, 0xbe, 0x97, 0xe8, 0xaf, 0xa5, 0xe8,
	0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe6, 0xba, 0x90, 0xe7, 0xa0, 0x81, 0xe4,
	0xb8, 0xba, 0xe8, 0xbe, 0xbe, 0xe6, 0x88, 0x90, 0xe6, 0xad, 0xa4, 0xe7,
	0x9b, 0xae, 0xe7, 0x9a, 0x84, 0xe4, 0xb9, 0x8b, 0xe5, 0x89, 0x8d, 0xe6,
	0x8f, 0x90, 0xe3, 0x80, 0x82, 0x0a, 0xe7, 0x9b, 0xb8, 0xe6, 0xaf, 0x94,
	0x31, 0x39, 0x38, 0x33, 0xe5, 0xb9, 0xb4, 0xef, 0xbc, 0x8c, 0xe6, 0x8a,
	0x80, 0xe6, 0x9c, 0xaf, 0xe5, 0x92, 0x8c, 0xe7, 0xbd, 0x91, 0xe7, 0xbb,
	0x9c, 0xe7, 0x9a, 0x84, 0xe5, 0x8f, 0x91, 0xe5, 0xb1, 0x95, 0xe4, 0xbd,
	0xbf, 0xe8, 0xbf, 0x99, 0xe4, 0xba, 0x9b, 0xe8, 0x87, 0xaa, 0xe7, 0x94,
	0xb1, 0xe7, 0x8e, 0xb0, 0xe5, 0x9c, 0xa8, 0xe6, 0x9b, 0xb4, 0xe5, 0x8a,
	0xa0, 0xe9, 0x87, 0x8d, 0xe8, 0xa6, 0x81, 0xe3, 0x80, 0x82, 0x0a, 0x0a,
	0xe7, 0x8e, 0xb0, 0xe5, 0x9c, 0xa8, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1,
	0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe8, 0xbf, 0x90, 0xe5, 0x8a, 0xa8,
	0xe8, 0xbf, 0x9c, 0xe8, 0xbf, 0x9c, 0xe4, 0xb8, 0x8d, 0xe4, 0xbb, 0x85,
	0xe6, 0x98, 0xaf, 0xe5, 0xbc, 0x80, 0xe5, 0x8f, 0x91, 0x20, 0x47, 0x4e,
	0x55, 0x20, 0xe7, 0xb3, 0xbb, 0xe7, 0xbb, 0x9f, 0xe3, 0x80, 0x82, 0xe5,
	0x8f, 0x82, 0xe7, 0x9c, 0x8b, 0x20, 0xe8, 0x87, 0xaa, 0xe7, 0x94, 0xb1,
	0xe8, 0xbd, 0xaf, 0xe4, 0xbb, 0xb6, 0xe5, 0x9f, 0xba, 0xe9, 0x87, 0x91,
	0xe4, 0xbc, 0x9a, 0xe7, 0xbd, 0x91, 0xe7, 0xab, 0x99, 0x20, 0xe6, 0x9d,
	0xa5, 0xe4, 0xba, 0x86, 0xe8, 0xa7, 0xa3, 0xe6, 0x9b, 0xb4, 0xe5, 0xa4,
	0x9a, 0xe5, 0x85, 0xb3, 0xe4, 0xba, 0x8e, 0xe6, 0x88, 0x91, 0xe4, 0xbb,
	0xac, 0xe5, 0x81, 0x9a, 0xe4, 0xbb, 0x80, 0xe4, 0xb9, 0x88, 0xef, 0xbc,
	0x8c, 0xe4, 0xbb, 0xa5, 0xe5, 0x8f, 0x8a, 0x20, 0xe4, 0xbd, 0xa0, 0xe5,
	0x8f, 0xaf, 0xe4, 0xbb, 0xa5, 0xe5, 0x8f, 0x82, 0xe4, 0xb8, 0x8e, 0x20,
	0xe7, 0x9a, 0x84, 0xe6, 0x96, 0xb9, 0xe5, 0xbc, 0x8f, 0xe3, 0x80, 0x82,
	0x0a
};

uint8_t utf16_text[] = {
	0xea, 0x81, 0x31, 0x75, 0x6f, 0x8f, 0xf6, 0x4e, 0x73, 0x51, 0x4e, 0x4e,
	0xea, 0x81, 0x31, 0x75, 0x0c, 0xff, 0x0c, 0x80, 0x5e, 0x97, 0xf7, 0x4e,
	0x3c, 0x68, 0x02, 0x30, 0x81, 0x89, 0x06, 0x74, 0xe3, 0x89, 0xd9, 0x8f,
	0x2a, 0x4e, 0x82, 0x69, 0xf5, 0x5f, 0x0c, 0xff, 0x60, 0x4f, 0x94, 0x5e,
	0xe5, 0x8b, 0x03, 0x80, 0x51, 0x86, 0x20, 0x00, 0x1c, 0x20, 0x66, 0x00,
	0x72, 0x00, 0x65, 0x00, 0x65, 0x00, 0x1d, 0x20, 0x20, 0x00, 0x2f, 0x66,
	0x20, 0x00, 0x1c, 0x20, 0x00, 0x8a, 0xba, 0x8b, 0xea, 0x81, 0x31, 0x75,
	0x08, 0xff, 0x66, 0x00, 0x72, 0x00, 0x65, 0x00, 0x65, 0x00, 0x20, 0x00,
	0x73, 0x00, 0x70, 0x00, 0x65, 0x00, 0x65, 0x00, 0x63, 0x00, 0x68, 0x00,
	0x09, 0xff, 0x1d, 0x20, 0x2d, 0x4e, 0x84, 0x76, 0x1c, 0x20, 0xea, 0x81,
	0x31, 0x75, 0x1d, 0x20, 0x1b, 0xff, 0x0c, 0x80, 0x0d, 0x4e, 0x2f, 0x66,
	0x20, 0x00, 0x1c, 0x20, 0x4d, 0x51, 0x39, 0x8d, 0x64, 0x55, 0x52, 0x91,
	0x08, 0xff, 0x66, 0x00, 0x72, 0x00, 0x65, 0x00, 0x65, 0x00, 0x20, 0x00,
	0x62, 0x00, 0x65, 0x00, 0x65, 0x00, 0x72, 0x00, 0x09, 0xff, 0x1d, 0x20,
	0x2d, 0x4e, 0x84, 0x76, 0x1c, 0x20, 0x4d, 0x51, 0x39, 0x8d, 0x1d, 0x20,
	0x02, 0x30, 0x0a, 0x00, 0x0a, 0x00, 0xf4, 0x66, 0xbe, 0x7c, 0x6e, 0x78,
	0x30, 0x57, 0xf4, 0x8b, 0x0c, 0xff, 0xea, 0x81, 0x31, 0x75, 0x6f, 0x8f,
	0xf6, 0x4e, 0x4b, 0x8d, 0x88, 0x4e, 0x6f, 0x8f, 0xf6, 0x4e, 0x7f, 0x4f,
	0x28, 0x75, 0x05, 0x80, 0xdb, 0x56, 0x79, 0x98, 0xfa, 0x57, 0x2c, 0x67,
	0xea, 0x81, 0x31, 0x75, 0x1a, 0xff, 0x0a, 0x00, 0x0a, 0x00, 0x0d, 0x4e,
	0xba, 0x8b, 0xee, 0x76, 0x84, 0x76, 0x3a, 0x4e, 0x55, 0x4f, 0x0c, 0xff,
	0x09, 0x67, 0xd0, 0x8f, 0x4c, 0x88, 0xe5, 0x8b, 0x6f, 0x8f, 0xf6, 0x4e,
	0x84, 0x76, 0xea, 0x81, 0x31, 0x75, 0x08, 0xff, 0xea, 0x81, 0x31, 0x75,
	0x4b, 0x4e, 0xf6, 0x96, 0x09, 0xff, 0x02, 0x30, 0x0a, 0x00, 0x09, 0x67,
	0x14, 0x78, 0x76, 0x7a, 0xe5, 0x8b, 0x6f, 0x8f, 0xf6, 0x4e, 0x82, 0x59,
	0x55, 0x4f, 0xe5, 0x5d, 0x5c, 0x4f, 0xe5, 0x4e, 0xca, 0x53, 0x09, 0x63,
	0x00, 0x97, 0x39, 0x65, 0x99, 0x51, 0xe5, 0x8b, 0x6f, 0x8f, 0xf6, 0x4e,
	0x84, 0x76, 0xea, 0x81, 0x31, 0x75, 0x08, 0xff, 0xea, 0x81, 0x31, 0x75,
	0x4b, 0x4e, 0x00, 0x4e, 0x09, 0xff, 0x02, 0x30, 0xd6, 0x53, 0x97, 0x5f,
	0xe5, 0x8b, 0x6f, 0x8f, 0xf6, 0x4e, 0x90, 0x6e, 0xe3, 0x4e, 0x01, 0x78,
	0x3a, 0x4e, 0xbe, 0x8f, 0x10, 0x62, 0x64, 0x6b, 0xee, 0x76, 0x84, 0x76,
	0x4b, 0x4e, 0x4d, 0x52, 0xd0, 0x63, 0x02, 0x30, 0x0a, 0x00, 0x09, 0x67,
	0xcd, 0x91, 0xb0, 0x65, 0xd1, 0x53, 0x03, 0x5e, 0xf7, 0x62, 0x1d, 0x8d,
	0x84, 0x76, 0xea, 0x81, 0x31, 0x75, 0x0c, 0xff, 0xd9, 0x8f, 0x37, 0x68,
	0x60, 0x4f, 0xef, 0x53, 0xe5, 0x4e, 0x1f, 0x50, 0x64, 0x6b, 0x65, 0x67,
	0x66, 0x65, 0xb2, 0x4e, 0x66, 0x77, 0xbb, 0x90, 0x08, 0xff, 0xea, 0x81,
	0x31, 0x75, 0x4b, 0x4e, 0x8c, 0x4e, 0x09, 0xff, 0x02, 0x30, 0x0a, 0x00,
	0x09, 0x67, 0x11, 0x54, 0x6c, 0x51, 0x17, 0x4f, 0xd1, 0x53, 0x03, 0x5e,
	0x39, 0x65, 0xdb, 0x8f, 0x48, 0x72, 0x6f, 0x8f, 0xf6, 0x4e, 0x84, 0x76,
	0xea, 0x81, 0x31, 0x75, 0x08, 0xff, 0xea, 0x81, 0x31, 0x75, 0x4b, 0x4e,
	0x09, 0x4e, 0x09, 0xff, 0x0c, 0xff, 0xd9, 0x8f, 0x37, 0x68, 0x74, 0x65,
	0x2a, 0x4e, 0x3e, 0x79, 0xa4, 0x7f, 0xfd, 0x90, 0xef, 0x53, 0xe0, 0x56,
	0x64, 0x6b, 0xd7, 0x53, 0xe0, 0x60, 0x02, 0x30, 0xd6, 0x53, 0x97, 0x5f,
	0xe5, 0x8b, 0x6f, 0x8f, 0xf6, 0x4e, 0x90, 0x6e, 0x01, 0x78, 0x3a, 0x4e,
	0xbe, 0x8f, 0x10, 0x62, 0x64, 0x6b, 0xee, 0x76, 0x84, 0x76, 0x4b, 0x4e,
	0x4d, 0x52, 0xd0, 0x63, 0x02, 0x30, 0x0a, 0x00, 0xf8, 0x76, 0xd4, 0x6b,
	0x31, 0x00, 0x39, 0x00, 0x38, 0x00, 0x33, 0x00, 0x74, 0x5e, 0x0c, 0xff,
	0x80, 0x62, 0x2f, 0x67, 0x8c, 0x54, 0x51, 0x7f, 0xdc, 0x7e, 0x84, 0x76,
	0xd1, 0x53, 0x55, 0x5c, 0x7f, 0x4f, 0xd9, 0x8f, 0x9b, 0x4e, 0xea, 0x81,
	0x31, 0x75, 0xb0, 0x73, 0x28, 0x57, 0xf4, 0x66, 0xa0, 0x52, 0xcd, 0x91,
	0x81, 0x89, 0x02, 0x30, 0x0a, 0x00, 0x0a, 0x00, 0xb0, 0x73, 0x28, 0x57,
	0xea, 0x81, 0x31, 0x75, 0x6f, 0x8f, 0xf6, 0x4e, 0xd0, 0x8f, 0xa8, 0x52,
	0xdc, 0x8f, 0xdc, 0x8f, 0x0d, 0x4e, 0xc5, 0x4e, 0x2f, 0x66, 0x00, 0x5f,
	0xd1, 0x53, 0x20, 0x00, 0x47, 0x00, 0x4e, 0x00, 0x55, 0x00, 0x20, 0x00,
	0xfb, 0x7c, 0xdf, 0x7e, 0x02, 0x30, 0xc2, 0x53, 0x0b, 0x77, 0x20, 0x00,
	0xea, 0x81, 0x31, 0x75, 0x6f, 0x8f, 0xf6, 0x4e, 0xfa, 0x57, 0xd1, 0x91,
	0x1a, 0x4f, 0x51, 0x7f, 0xd9, 0x7a, 0x20, 0x00, 0x65, 0x67, 0x86, 0x4e,
	0xe3, 0x89, 0xf4, 0x66, 0x1a, 0x59, 0x73, 0x51, 0x8e, 0x4e, 0x11, 0x62,
	0xec, 0x4e, 0x5a, 0x50, 0xc0, 0x4e, 0x48, 0x4e, 0x0c, 0xff, 0xe5, 0x4e,
	0xca, 0x53, 0x20, 0x00, 0x60, 0x4f, 0xef, 0x53, 0xe5, 0x4e, 0xc2, 0x53,
	0x0e, 0x4e, 0x20, 0x00, 0x84, 0x76, 0xb9, 0x65, 0x0f, 0x5f, 0x02, 0x30,
	0x0a, 0x00
};


static void utf_8to16(ut_runner *r)
{

	size_t need_len = ax_utf8_to_utf16((char *)utf8_text, sizeof utf8_text, NULL, 0);
	ut_assert_uint_equal(r, sizeof utf16_text / 2, need_len);

	uint16_t *out = malloc(need_len * 2);
	if (!out)
		ut_fail(r, "malloc");

	size_t out_len = ax_utf8_to_utf16((char *)utf8_text, sizeof utf8_text, out, need_len);
	ut_assert_uint_equal(r, sizeof utf16_text / 2, out_len);

	for (int i = 0; i < out_len; i++)
		ut_assert_int_equal(r, ((uint16_t *)utf16_text)[i], out[i]);
}

static void utf_16to8(ut_runner *r)
{
	size_t need_len = ax_utf16_to_utf8((uint16_t *)utf16_text, sizeof utf16_text / 2, NULL, 0);
	ut_assert_uint_equal(r, sizeof utf8_text, need_len);

	char *out = malloc(need_len);
	if (!out)
		ut_fail(r, "malloc");

	size_t out_len = ax_utf16_to_utf8((uint16_t *)utf16_text, sizeof utf16_text, out, need_len);
	ut_assert_uint_equal(r, sizeof utf8_text, out_len);

	for (int i = 0; i < out_len; i++)
		ut_assert_int_equal(r, utf8_text[i], (uint8_t)out[i]);
}

ut_suite *suite_for_unicode()
{
	ut_suite* suite = ut_suite_create("unicode");

	ut_suite_add(suite, utf_8to16, 0);

	ut_suite_add(suite, utf_16to8, 0);

	return suite;
}
