/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
 * 
 * Permission is hereby granted, free of charge, to one person obtaining a copy
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

#include "ax/lib.h"
#include <ax/detect.h>

#if defined(AX_OS_WIN32)
#include <windef.h>
#include <winbase.h>
#include <libloaderapi.h>
#include <errhandlingapi.h>
#else
#include <dlfcn.h>
#endif

#include <stdio.h>

#if defined(AX_OS_WIN32)
static wchar_t last_error_message_win32[256];
static void ax_lib_win32_seterror(void)
{
	DWORD errcode = GetLastError();
	if (!FormatMessageW(FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, errcode, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
			last_error_message_win32, sizeof(last_error_message_win32) / 2 - 1, NULL)) {
		wsprintfW(last_error_message_win32, L"unknown error %lu", errcode);
	}
}

#endif

ax_lib *ax_lib_open(const ax_uchar* fname)
{
#if defined(AX_OS_WIN32)
	HMODULE h;
	int emd;
	emd = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	h = LoadLibraryW(fname);
	SetErrorMode(emd);
	if(!h) {
		ax_lib_win32_seterror();
		return NULL;
	}
	last_error_message_win32[0] = 0;
	return (ax_lib *)h;
#else
	return dlopen(fname, RTLD_NOW | RTLD_GLOBAL);
#endif
}

void *ax_lib_symbol(ax_lib *lib, const char *func)
{
#if defined(AX_OS_WIN32)
	void *ptr;
	*(FARPROC*)(&ptr) = GetProcAddress((HMODULE)lib, func);
	if(!ptr) {
		ax_lib_win32_seterror();
		return NULL;
	}
	last_error_message_win32[0] = 0;
	return ptr;
#else
	return dlsym(lib, func);
#endif
}

int ax_lib_close(ax_lib *lib)
{
	if (!lib)
		return 0;
#if defined(AX_OS_WIN32)
	if (!FreeLibrary((HMODULE)lib)) {
		ax_lib_win32_seterror();
		return -1;
	}
	last_error_message_win32[0] = 0;
	return 0;
#else
	return - !!dlclose(lib);
#endif
}

const char *ax_lib_error(void)
{
#if defined(AX_OS_WIN32)
	if (last_error_message_win32[0])
		return last_error_message_win32;
	else
		return NULL;
#else
	return dlerror();
#endif
}
