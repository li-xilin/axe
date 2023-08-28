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

#include "ax/sys.h"
#include "ax/types.h"
#include "ax/detect.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

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
		switch (GetLastError()) {
			case ERROR_ALREADY_EXISTS:
				errno = EEXIST;
				break;
			case ERROR_PATH_NOT_FOUND:
				errno = ENOENT;
				break;
		}
		return -1;
	}
	return 0;
#else
	return mkdir(path, mode);
#endif
}

int ax_sys_unlink(const ax_uchar *path)
{
#ifdef AX_OS_WIN
	if (!DeleteFileW(path)) {
		switch (GetLastError()) {
			case ERROR_FILE_NOT_FOUND:
				errno = ENOENT;
				break;
			case ERROR_ACCESS_DENIED:
				errno = EPERM;
				break;
			default:
				errno = EINVAL;
		}
		return -1;
	}
	return 0;
#else
	return unlink(path);
#endif
}

int ax_sys_rename(const ax_uchar *path, const ax_uchar *new_path)
{
#ifdef AX_OS_WIN
	if (!MoveFileW(path, new_path)) {
		switch (GetLastError()) {
			case ERROR_FILE_NOT_FOUND:
				errno = ENOENT;
				break;
			case ERROR_ACCESS_DENIED:
				errno = EPERM;
				break;
			case ERROR_ALREADY_EXISTS:
				errno = EEXIST;
				break;
			default:
				errno = EINVAL;
		}
	}
	return 0;
#else
	return rename(path, new_path);
#endif
}

int ax_sys_copy(const ax_uchar *path, const ax_uchar *target_path)
{
	errno = ENOTSUP;
	return -1;
}

int ax_sys_link(const ax_uchar *path, const ax_uchar *target_path)
{
	errno = ENOTSUP;
	return -1;
}

int ax_sys_access(const ax_uchar *path, int mode)
{
	errno = ENOTSUP;
	return -1;
}

FILE *ax_sys_fopen(const ax_uchar *path, const ax_uchar *mode)
{
#ifdef AX_OS_WIN
	return _wfopen(path, mode);
#else
	return fopen(path, mode);
#endif
}

