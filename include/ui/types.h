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


#ifndef UI_TYPES_H
#define UI_TYPES_H

#include <stdint.h>

#ifndef UI_SIZE_DEFINED
#define UI_SIZE_DEFINED
typedef struct ui_size_st ui_size;
#endif

struct ui_size_st
{
	int width, height;
};

#ifndef UI_RECT_DEFINED
#define UI_RECT_DEFINED
typedef struct ui_rect_st ui_rect;
#endif

struct ui_rect_st
{
	int left, top, right, bottem;
};

#ifndef UI_POINT_DEFINED
#define UI_POINT_DEFINED
typedef struct ui_point_st ui_point;
#endif

struct ui_point_st
{
	int x, y;
};

#ifndef UI_RGBA_DEFINED
#define UI_RGBA_DEFINED
typedef struct ui_rgba_st ui_rgba;
#endif

struct ui_rgba_st
{
	uint8_t r, g, b, a;
};

#endif

