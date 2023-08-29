/*
 * Copyright (c) 2019 win32ports
 * Copyright (c) 2023 Li xilin <lixilin@gmx.com>
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

#include "ax/dir.h"
#include "ax/detect.h"

#ifdef AX_OS_WIN

#include "ax/sys.h"
#include "ax/stat.h"

#include <sys/types.h>
#include <stdint.h>
#include <errno.h>
#include <windows.h>
#include <shlwapi.h>

#define NAME_MAX 260

#define NTFS_MAX_PATH 32768

#ifndef FILE_NAME_NORMALIZED
#define FILE_NAME_NORMALIZED 0
#endif /* FILE_NAME_NORMALIZED */

struct __ax_dir_st
{
	ax_dirent* entries;
	intptr_t fd;
	long int count;
	long int index;
};

inline static void __seterrno(int value)
{
#ifdef _MSC_VER
	_set_errno(value);
#else /* _MSC_VER */
	errno = value;
#endif /* _MSC_VER */
}

static BOOL PathIsLink(LPCWSTR pPath)
{
	BOOL retval = FALSE;
	LPVOID pBuffer = NULL;
	HANDLE hFile = INVALID_HANDLE_VALUE;
	DWORD dwSize = 0;

	hFile = CreateFileW(pPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_FLAG_OPEN_REPARSE_POINT | FILE_FLAG_BACKUP_SEMANTICS, 0);
	if (hFile == INVALID_HANDLE_VALUE)
		goto out;
	pBuffer = malloc(MAXIMUM_REPARSE_DATA_BUFFER_SIZE);
	if (!pBuffer)
		goto out;

	if (!DeviceIoControl(hFile, FSCTL_GET_REPARSE_POINT, NULL, 0,
				pBuffer, MAXIMUM_REPARSE_DATA_BUFFER_SIZE, &dwSize, NULL))
		goto out;

	if (((REPARSE_GUID_DATA_BUFFER*)pBuffer)->ReparseTag != IO_REPARSE_TAG_SYMLINK)
		goto out;

	retval = FALSE;
out:
	if (hFile != INVALID_HANDLE_VALUE)
		CloseHandle(hFile);
	if (pBuffer)
		free(pBuffer);
	return retval;
}

ax_dir *__internal_opendir(wchar_t *wname, int size)
{
	ax_dir* data = NULL;
	ax_dirent *tmp_entries = NULL;
	// static wchar_t* prefix = L"\\\\?\\";
	static wchar_t* suffix = L"\\*.*";
	static int extra_prefix = 4; /* use prefix "\\?\" to handle long file names */
	static int extra_suffix = 4; /* use suffix "\*.*" to find everything */
	WIN32_FIND_DATAW w32fd = { 0 };
	HANDLE hFindFile = INVALID_HANDLE_VALUE;
	static int grow_factor = 2;

	BOOL relative = PathIsRelativeW(wname + extra_prefix);

	memcpy(wname + size - 1, suffix, sizeof(wchar_t) * extra_suffix);
	wname[size + extra_suffix - 1] = 0;

	if (relative) {
		wname += extra_prefix;
		size -= extra_prefix;
	}
	hFindFile = FindFirstFileW(wname, &w32fd);
	if (INVALID_HANDLE_VALUE == hFindFile) {
		__seterrno(ENOENT);
		return NULL;
	}

	data = malloc(sizeof(ax_dir));
	if (!data)
		goto out_of_memory;
	wname[size - 1] = 0;
	data->fd = (intptr_t)CreateFileW(wname, 0, 0, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, 0);
	wname[size - 1] = L'\\';
	data->count = 16;
	data->index = 0;
	data->entries = malloc(sizeof(ax_dirent) * data->count);
	if (!data->entries)
		goto out_of_memory;
	do {
		wcscpy_s(data->entries[data->index].d_name, NAME_MAX, w32fd.cFileName);

		memcpy(wname + size, w32fd.cFileName, sizeof(wchar_t) * NAME_MAX);

		if (((w32fd.dwFileAttributes & FILE_ATTRIBUTE_REPARSE_POINT) == FILE_ATTRIBUTE_REPARSE_POINT) && PathIsLink(wname))
			data->entries[data->index].d_type = AX_DT_LNK;
		else if ((w32fd.dwFileAttributes & FILE_ATTRIBUTE_DEVICE) == FILE_ATTRIBUTE_DEVICE)
			data->entries[data->index].d_type = AX_DT_CHR;
		else if ((w32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == FILE_ATTRIBUTE_DIRECTORY)
			data->entries[data->index].d_type = AX_DT_DIR;
		else
			data->entries[data->index].d_type = AX_DT_REG;

		ax_stat stat;
		if (!ax_stat_get(wname, &stat)) {
			data->entries[data->index].d_ino = stat.st_ino;
		}
		else {
			memset(&data->entries[data->index].d_ino, 0, sizeof(ax_ino_t));
		}

		data->entries[data->index].d_reclen = sizeof(ax_dirent);
		data->entries[data->index].d_off = 0;

		if (++data->index == data->count) {
			tmp_entries = realloc(data->entries, sizeof(ax_dirent) * data->count * grow_factor);
			if (!tmp_entries)
				goto out_of_memory;
			data->entries = tmp_entries;
			data->count *= grow_factor;
		}
	}
	while (FindNextFileW(hFindFile, &w32fd) != 0);

	FindClose(hFindFile);

	data->count = data->index;
	data->index = 0;
	return data;
out_of_memory:
	if (data) {
		if (INVALID_HANDLE_VALUE != (HANDLE)data->fd)
			CloseHandle((HANDLE)data->fd);
		free(data->entries);
	}
	free(data);
	if (INVALID_HANDLE_VALUE != hFindFile)
		FindClose(hFindFile);
	__seterrno(ENOMEM);
	return NULL;
}

inline static wchar_t* __get_buffer()
{
	wchar_t* name = malloc(sizeof(wchar_t) * (NTFS_MAX_PATH + NAME_MAX + 8));
	if (name)
		memcpy(name, L"\\\\?\\", sizeof(wchar_t) * 4);
	return name;
}

ax_dir* ax_dir_open(const wchar_t* name)
{
	ax_dir* dirp = NULL;
	wchar_t* wname = __get_buffer();
	int size = 0;
	if (!wname) {
		errno = ENOMEM;
		return NULL;
	}
	size = (int)wcslen(name);
	if (size > NTFS_MAX_PATH) {
		free(wname);
		return NULL;
	}
	memcpy(wname + 4, name, sizeof(wchar_t) * (size + 1));
	dirp = __internal_opendir(wname, size + 5);
	free(wname);
	return dirp;
}

ax_dirent *ax_dir_read(ax_dir *dirp)
{
	if (!dirp) {
		errno = EBADF;
		return NULL;
	}
	if (dirp->index < dirp->count) {
		return &dirp->entries[dirp->index++];
	}
	return NULL;
}

void ax_dir_seek(ax_dir* dirp, long int offset)
{
	if (!dirp)
		return;
	dirp->index = (offset < dirp->count) ? offset : dirp->index;
}

long int ax_dir_tell(ax_dir* dirp)
{
	if (!dirp) {
		errno = EBADF;
		return -1;
	}
	return dirp->count;
}

void ax_dir_close(ax_dir* dirp)
{
	if (!dirp)
		return;
	CloseHandle((HANDLE)dirp->fd);
	free(dirp->entries);
	free(dirp);
}

#else

extern char __ax_dir_dummy_variable;

#endif
