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

#ifndef AX_UCHAR_H
#define AX_UCHAR_H

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <ax/detect.h>
#include <ax/unicode.h>
#include <ax/mem.h>

#ifdef AX_OS_WIN

#include <windef.h>
#include <winbase.h>
#include <shellapi.h>
typedef WCHAR ax_uchar;
#define __ax_u(s) L##s
#define ax_ustrlen wcslen
#define ax_ustrcat wcscat
#define ax_ustrcpy wcscpy
#define ax_ustrncpy wcsncpy
#define ax_ustrstr wcsstr
#define ax_ustrchr wcschr
#define ax_ustrrchr wcsrchr
#define ax_ustrcmp wcscmp
#define ax_ustricmp _wcsicmp
#define ax_ustrnicmp _wcsnicmp
#define ax_ustrncmp wcsncmp
#define ax_usscanf swscanf
#define ax_usnprintf swprintf
#define ax_usprintf _swprintf
#define ax_uvsnprintf vsnwprintf
#define ax_ustrhash ax_wcshash
#define ax_ustrargv ax_wcsargv
#define ax_ustrtrim ax_wcstrim
#define ax_ustr_index ax_utf16_index
#define ax_ustr_to_ucode ax_utf16_to_ucode
#define ax_ustr_charcnt ax_utf16_charcnt
#define ax_ucode_to_ustr ax_ucode_to_utf16
#define ax_ustrtoint _wtoi

#define AX_PRIus "ls"
#define AX_PRIs "hs"
#define AX_PRIuc "lc"
#define AX_PRIc "hc"

#define ax_t_ustr ax_t_wcs
#define AX_UCHAR_LEN 2

#else

typedef char ax_uchar;
#define __ax_u(s) s
#define ax_ustrlen strlen
#define ax_ustrcat strcat
#define ax_ustrcpy strcpy
#define ax_ustrncpy strncpy
#define ax_ustrstr strstr
#define ax_ustrchr strchr
#define ax_ustrrchr strrchr
#define ax_ustrcmp strcmp
#define ax_ustrncmp strncmp
#define ax_ustricmp strcasecmp
#define ax_ustrnicmp strncasecmp
#define ax_usscanf sscanf
#define ax_usprintf sprintf
#define ax_usnprintf snprintf
#define ax_uvsnprintf vsnprintf
#define ax_ustrhash ax_strhash
#define ax_ustrargv ax_strargv
#define ax_ustrtrim ax_strtrim
#define ax_ustr_index ax_utf8_index
#define ax_ustr_to_ucode ax_utf8_to_ucode
#define ax_ustr_charcnt ax_utf8_charcnt
#define ax_ucode_to_ustr ax_ucode_to_utf8
#define ax_ustrtoint atoi

#define AX_PRIus "s"
#define AX_PRIs "s"
#define AX_PRIuc "c"
#define AX_PRIc "c"
#define ax_t_ustr ax_t_str
#define AX_UCHAR_LEN 1

#endif

#define AX_US(flag) "%" #flag AX_PRIus

#define ax_u(s) __ax_u(s)

typedef int ax_umain_f(int argc, ax_uchar *argv[]);

int ax_ustr_from_utf8(ax_uchar *us, size_t size, const char *from);

int ax_ustr_from_ansi(ax_uchar *us, size_t size, const char *from);

int ax_ustr_from_utf16(ax_uchar *us, size_t size, const uint16_t *from);

int ax_ustr_utf8(const ax_uchar *us, char *to, size_t size);

int ax_ustr_ansi(const ax_uchar *us, char *to, size_t size);

int ax_ustr_utf16(const ax_uchar *us, uint16_t *to, size_t size);

ax_uchar *ax_ustrsplit(ax_uchar **s, ax_uchar ch);

ax_uchar *ax_ustrdup(const ax_uchar *s);

size_t ax_ustrihash(const ax_uchar *s);

size_t ax_ustrnihash(const ax_uchar *s, size_t len);


inline static int ax_umain(ax_umain_f *umain, int argc, char *argv[])
{
#ifdef AX_OS_WIN
	int nArgs;
	LPWSTR *pArgList = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (!pArgList)
		return 1;
	return umain(nArgs, pArgList);
#else
	return umain(argc, argv);
#endif
}

#define ax_main \
	main(int argc, char *argv[]) { \
		extern int ax_main(int argc, ax_uchar *argv[]); \
		return ax_umain(ax_main, argc, argv);  \
	} \
	extern int ax_main

#endif
