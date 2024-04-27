/*
 * Copyright (c) 2023-2024 Li Xilin <lixilin@gmx.com>
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

#include "ax/io.h"
#include "ax/def.h"
#include "ax/sys.h"
#include "ax/errno.h"
#include "ax/types.h"
#include "ax/detect.h"
#include "ax/once.h"
#include "ax/tss.h"
#include "ax/mutex.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef AX_OS_WIN
#include <windows.h>
#include <fileapi.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <utime.h>
#include <sys/types.h>
#include <sys/sysinfo.h>
#endif

int ax_sys_mkdir(const ax_uchar *path, int mode)
{
#ifdef AX_OS_WIN
	if (!CreateDirectoryW(path, 0)) {
#else
	if (mkdir(path, mode)) {
#endif
		ax_error_occur();
		return -1;
	}
	return 0;
}

int ax_sys_unlink(const ax_uchar *path)
{
#ifdef AX_OS_WIN
	if (!DeleteFileW(path)) {
#else
	if (unlink(path)) {
#endif
		ax_error_occur();
		return -1;
	}
	return 0;
}

int ax_sys_rename(const ax_uchar *path, const ax_uchar *new_path)
{
#ifdef AX_OS_WIN
	if (!MoveFileW(path, new_path)) {
		
#else
	if (rename(path, new_path)) {
#endif
		ax_error_occur();
		return -1;
	}
	return 0;
}

int ax_sys_copy(const ax_uchar *path, const ax_uchar *new_path)
{
	
	int retval = -1;
#ifdef AX_OS_WIN
	if (!CopyFileW(path, new_path, FALSE)) {
		ax_error_occur();
		return -1;
	}
	retval = 0;
#else
	FILE *from_fp = NULL, *to_fp = NULL;

       	if (!(from_fp = ax_fopen(path, ax_u("rb"))))
		goto out;

       	if (!(to_fp = ax_fopen(new_path, ax_u("wb"))))
		goto out;

	ssize_t len;
	char buf[4096];
	while ((len = fread(buf, 1, sizeof buf, from_fp))) {
		if (fwrite(buf, 1, len, to_fp) < len)
			break;
	}

	if (ferror(from_fp) || ferror(to_fp)) {
		ax_sys_unlink(new_path);
		errno = AX_EIO;
		goto out;
	}

	retval = 0;
out:
	if (from_fp)
		fclose(from_fp);
	if (to_fp)
		fclose(to_fp);
#endif
	return retval;
}

int ax_sys_link(const ax_uchar *path, const ax_uchar *link_path)
{
#ifdef AX_OS_WIN
	if (!CreateHardLinkW(link_path, path, NULL)) {
#else
	if (link(path, link_path)) {
#endif
		ax_error_occur();
		return -1;
	}
	return 0;
}

int ax_sys_symlink(const ax_uchar *path, const ax_uchar *link_path, bool dir_link)
{
#ifdef AX_OS_WIN
	DWORD dwFlags = 0;
	if (dir_link)
		dwFlags |= SYMBOLIC_LINK_FLAG_DIRECTORY;
	if (!CreateSymbolicLinkW(link_path, path, dwFlags)) {
#else
	if (symlink(path, link_path)) {
#endif
		ax_error_occur();
		return -1;
	}
	return 0;
}

static ax_once s_env_once = AX_ONCE_INIT;
static ax_tss s_env_tss;
static ax_mutex s_env_lock = AX_MUTEX_INIT;

static void getcwd_tss_free(void *p)
{
        free(p);
}

static void getcwd_init(void)
{
        ax_tss_create(&s_env_tss, getcwd_tss_free);
}

const ax_uchar *ax_sys_getenv(const ax_uchar *name)
{
	assert(name);

	ax_uchar *tss_buf = NULL, *val;
        if (ax_once_run(&s_env_once, getcwd_init))
		goto out;

	if (ax_mutex_lock(&s_env_lock))
		goto out;
#ifdef AX_OS_WIN
	val = _wgetenv(name);
#else
	val = getenv(name);
#endif
        if (!val)
		goto unlock;
	if (!(tss_buf = ax_ustrdup(val)))
		goto unlock;

	free(ax_tss_get(&s_env_tss));
	if (ax_tss_set(&s_env_tss, tss_buf)) {
		free(tss_buf);
		tss_buf = NULL;
		goto unlock;
	}
unlock:
	ax_mutex_unlock(&s_env_lock);
out:
	return tss_buf;
}

int ax_sys_setenv(const ax_uchar *name, const ax_uchar *value)
{
#ifdef AX_OS_WIN
	if (!SetEnvironmentVariableW(name, value)) {
		ax_error_occur();
		return -1;
	}
	return 0;
#else
	return setenv(name, value, 1);
#endif
}

#define TIMET_TO_ULINTEGER(timet) ((ULARGE_INTEGER) { \
        .QuadPart = (LONGLONG)timet * 10000000 + 116444736000000000 \
})

int ax_sys_utime(const ax_uchar *path, time_t atime, time_t mtime)
{
#ifdef AX_OS_WIN
        int retval = -1;
        ULARGE_INTEGER uiAccessTime =  TIMET_TO_ULINTEGER(atime);
        ULARGE_INTEGER uiModifyTime =  TIMET_TO_ULINTEGER(mtime);
        FILETIME ftAccess = {
                .dwHighDateTime = uiAccessTime.HighPart,
                .dwLowDateTime = uiAccessTime.LowPart
        };
        FILETIME ftModify = {
                .dwHighDateTime = uiModifyTime.HighPart,
                .dwLowDateTime = uiModifyTime.LowPart
        };

        HANDLE hFile = CreateFileW(path, 0, 0, NULL, OPEN_EXISTING, 0, NULL);
        if (hFile == INVALID_HANDLE_VALUE)
                return -1;
        if (!SetFileTime(hFile, NULL, &ftAccess, &ftModify))
                goto out;
        retval = 0;
out:
        CloseHandle(hFile);
        return retval;
#else
	return utime(path, ax_pstruct(struct utimbuf, .actime = atime, .modtime = mtime));
#endif
}

int ax_sys_nprocs(void)
{
#ifdef AX_OS_WIN
	SYSTEM_INFO sys;
	GetSystemInfo(&sys);
	return sys.dwNumberOfProcessors;
#else
	return get_nprocs();
#endif
}

