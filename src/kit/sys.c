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

#include "ax/sys.h"
#include "ax/stdio.h"
#include "ax/errno.h"
#include "ax/types.h"
#include "ax/detect.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <stdbool.h>

#ifdef AX_OS_WIN
#include <windows.h>
#else
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
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
	FILE *from_fp = NULL;
	FILE *to_fp = NULL;

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

