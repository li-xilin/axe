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
static char last_error_message_win32[256];
static void ax_lib_win32_seterror(void)
{
	DWORD errcode = GetLastError();
	if (!FormatMessage(FORMAT_MESSAGE_IGNORE_INSERTS|FORMAT_MESSAGE_FROM_SYSTEM,
			NULL, errcode, MAKELANGID(LANG_ENGLISH, SUBLANG_DEFAULT),
			last_error_message_win32, sizeof(last_error_message_win32)-1, NULL)) {
		sprintf(last_error_message_win32, "unknown error %lu", errcode);
	}
}

#endif

ax_lib *ax_lib_open(const char* fname)
{
#if defined(AX_OS_WIN32)
	HMODULE h;
	int emd;
	emd = SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOOPENFILEERRORBOX);
	h = LoadLibrary(fname);
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
	if(!ptr)
	{
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
