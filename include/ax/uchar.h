/*
 * Copyright (c) 2023 Li xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef UCHAR_H
#define UCHAR_H

#include <ax/detect.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

#ifdef AX_OS_WIN

#include <windef.h>
#include <winbase.h>
typedef WCHAR ax_uchar;
#define ax_u(s) L##s
#define ax_ustrlen wcslen
#define ax_ustrcat wcscat
#define ax_ustrcpy wcscpy
#define ax_ustrncpy wcsncpy
#define ax_ustrstr wcsstr
#define ax_ustrrchr wcsrchr
#define ax_ustrcmp wcscmp
#define ax_ustrncmp wcscmp
#define ax_usprintf swprintf
#define ax_uvsnprintf wvsnprintf
#define AX_PRIus L"ls"

#else

typedef char ax_uchar;
#define ax_u(s) s
#define ax_ustrlen strlen
#define ax_ustrcat strcat
#define ax_ustrcpy strcpy
#define ax_ustrncpy strncpy
#define ax_ustrstr strstr
#define ax_ustrrchr strrchr
#define ax_ustrcmp strcmp
#define ax_ustrncmp strncmp
#define ax_usprintf sprintf
#define ax_uvsnprintf vsnprintf
#define AX_PRIus "s"

#endif

int ax_ustr_from_utf8(ax_uchar *us, size_t size, const char *from);

int ax_ustr_from_ansi(ax_uchar *us, size_t size, const char *from);

int ax_ustr_from_utf16(ax_uchar *us, size_t size, const uint16_t *from);

int ax_ustr_utf8(const ax_uchar *us, char *to, size_t size);

int ax_ustr_ansi(const ax_uchar *us, char *to, size_t size);

int ax_ustr_utf16(const ax_uchar *us, uint16_t *to, size_t size);

ax_uchar *ax_ustrsplit(ax_uchar **s, ax_uchar ch);

#endif
