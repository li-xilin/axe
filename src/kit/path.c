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

#include "ax/path.h"
#include "ax/sys.h"
#include "ax/detect.h"
#include "ax/mem.h"

#include <errno.h>
#include <stdarg.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#ifdef AX_OS_WIN
#include <windows.h>
#include <wchar.h>
#else
#include <unistd.h>
#endif

static int path_root(const ax_uchar *path);
static int path_rtrim(ax_uchar *path);

static int path_rtrim(ax_uchar *path)
{
	int len = ax_ustrlen(path);
	for (int i = len - 1; i != -1; i--) {
		if (path[i] != AX_PATH_SEP_CHAR)
			break;
		len--;
	}
	path[len] = ax_u('\0');
	return len;
}

void ax_path_empty(ax_uchar *path)
{
	ax_ustrcpy(path, ax_u(""));
}

ax_uchar *ax_path_push(ax_uchar *path, size_t size, const ax_uchar *name)
{
	int path_len = ax_ustrlen(path);
	int name_len = ax_ustrlen(name);

	bool need_sep = true;
	if (path_len == 0)
		need_sep = false;
	if (path[path_len - 1] == AX_PATH_SEP_CHAR)
		need_sep = false;

	if (path_len + name_len + !!need_sep + 1 > size) {
		errno = ENOBUFS;
		return NULL;
	}
	if (need_sep)
		ax_ustrcpy(path + path_len, AX_PATH_SEP);
	ax_ustrcpy(path + path_len + !!need_sep, name);
	return path;
}

ax_uchar *ax_path_trim(ax_uchar *path)
{
	if (!path)
		return NULL;

	bool pre_is_sep = false;
	ax_uchar *front = path, *rear = path;

	while (*front) {
		if (*front == AX_PATH_SEP_CHAR) {
			if (pre_is_sep) {
				front++;
				continue;
			}
			pre_is_sep = true;
		}
		else
			pre_is_sep = false;
		*rear++ = *front++;
	}
	if (rear != path && rear[-1] == AX_PATH_SEP_CHAR)
		rear[-1] = ax_u('\0');
	else
		*rear = ax_u('\0');
	return path;
}

bool ax_path_is_absolute(const ax_uchar *path)
{
	return !!path_root(path);
}

const ax_uchar *ax_path_pop(ax_uchar *path)
{
	if (!path)
		return NULL;

	int len = path_rtrim(path);

	for (int i = len - 1; i != -1; i--) {
		if (path[i] == AX_PATH_SEP_CHAR) {
			path[i] = ax_u('\0');
			return path + i + 1;
		}
	}

	if (!path[0]) {
		return NULL;
	}

	memmove(path + 1, path, (len + 1) * sizeof(ax_uchar));
	path[0] = ax_u('\0');
	return path + 1;
}

ax_uchar *ax_path_join(ax_uchar *path, size_t size, ...)
{
	va_list ap;
	va_start(ap, size);
	ax_uchar *name = va_arg(ap, ax_uchar *);
	while (name) {
		if (!ax_path_push(path, size, name))
			return NULL;
		name = va_arg(ap, ax_uchar *);
	}
	return path;
}

ax_uchar *ax_path_fixsep(ax_uchar *path)
{
	if (!path)
		return NULL;
#ifdef AX_OS_WIN32
	for (int i = 0; path[i]; i++)
		if (path[i] == L'/')
			path[i] = L'\\';
#endif
	return path;
}

/* path must be trimed before */
static int path_root(const ax_uchar *path)
{
	if (path[0] == AX_PATH_SEP_CHAR)
		return 1;
#ifdef AX_OS_WIN
	if (isalpha(path[0]) && path[1] == L':' ) {
		if (path[2] == ax_u('\\'))
			return 3;
		else if (path[2] == ax_u('\0'))
			return 2;
	}
#endif
	return 0;
}

ax_uchar *ax_path_normalize(ax_uchar *path)
{
	if (!ax_path_trim(ax_path_fixsep(path)))
		return NULL;

	int root_len = path_root(path);
	ax_uchar *p = path + root_len;
	
	ax_uchar *nam_tab[AX_PATH_MAX / 2];
	int name_cnt = 0;

	/* Split the path sequence by / or \ */
	ax_uchar *name;
	while ((name = ax_ustrsplit(&p, AX_PATH_SEP_CHAR))) {
		nam_tab[name_cnt] = name;
		name_cnt++;
	}
	nam_tab[name_cnt] = NULL;

	/* Remove duplicate hard link . and .. , but which begin with relative path are essential */
	int front = 0, rear = 0;
	for ( ;nam_tab[front]; front++) {
		if (ax_ustrcmp(nam_tab[front], ax_u(".")) == 0)
			continue;

		else if (ax_ustrcmp(nam_tab[front], ax_u("..")) == 0) {
			if (root_len) {
				if (rear)
					rear--;
				continue;
			}
			else {
				if (rear && ax_ustrcmp(nam_tab[rear - 1], ax_u("..")) != 0) {
					rear--;
					continue;
				}
			}
		}
		nam_tab[rear++] = nam_tab[front];
	}
	nam_tab[rear] = NULL;

	/* Construct new path sequence by name segments */
	if (nam_tab[0]) {
		ax_ustrcpy(path + root_len, nam_tab[0]);
		for (int i = 1; nam_tab[i]; i++) {
			int len = ax_ustrlen(path + root_len);
			path[len] = AX_PATH_SEP_CHAR;
			ax_ustrcpy(path + len + 1, nam_tab[i]);
		}
	}

	return path;
}

ax_uchar *ax_path_realize(const ax_uchar *path, ax_uchar *resolved_path, size_t size)
{
#ifdef AX_OS_WIN
	DWORD dwLen = GetFullPathNameW(path, size, resolved_path, NULL);
	if (dwLen == 0) {
		return NULL;
	}
	return resolved_path;
#else
	char resolved_buf[PATH_MAX];
	if (!realpath(path, resolved_buf))
		return NULL;
	int len = strlen(resolved_buf);
	if (len + 1 > size) {
		errno = ENOBUFS;
		return NULL;
	}
	memcpy(resolved_path, resolved_buf, len + 1);
	return resolved_path;
#endif
}

const ax_uchar *ax_path_basename(const ax_uchar *path)
{
	int i;
	int len = ax_ustrlen(path);
	for (i = len - 1; i != -1; i--) {
		if (path[i] == AX_PATH_SEP_CHAR)
			return path + i;
	}
	return path;
}

const ax_uchar *ax_path_extname(const ax_uchar *path)
{
	int i;
	int len = ax_ustrlen(path);
	for (i = len - 1; i != -1; i--) {
		if (path[i] == AX_PATH_SEP_CHAR)
			return path + len;

		if (path[i] == ax_u('.'))
			break;
	}

	if (i > 0 && path[i - 1] != AX_PATH_SEP_CHAR)
		return path + i + 1;
	else
		return path + len;
}

ax_uchar *ax_path_getcwd(ax_uchar *path, size_t size)
{
#ifdef AX_OS_WIN
	return _wgetcwd(path, size);
#else
	return getcwd(path, size);
#endif
}

int ax_path_setcwd(const ax_uchar *path)
{
#ifdef AX_OS_WIN
	return _wchdir(path);
#else
	return chdir(path);
#endif
}

ax_uchar *ax_path_homedir(ax_uchar *path, size_t size)
{
	const ax_uchar *home_dir = ax_sys_getenv(
#ifdef AX_OS_WIN
	L"USERPROFILE"
#else
	"HOME"
#endif
	);
	if (!home_dir) {
		errno = ENOENT;
		return NULL;
	}
	int len = ax_ustrlen(home_dir);

	if (len + 1 > size) {
		errno = ENOBUFS;
		return NULL;
	}
	memcpy(path, home_dir, (len + 1) * sizeof(ax_uchar));
	return path;
}

ax_uchar *ax_path_tmpdir(ax_uchar *path, size_t size)
{
#ifdef AX_OS_WIN
	DWORD dwLen = GetTempPathW(size, path);
	if (dwLen == 0)
		return NULL;
#else
#define UNIX_TEMP_PATH "/tmp"
	if (size < sizeof UNIX_TEMP_PATH) {
		errno = ENOBUFS;
		return NULL;
	}
	
	strcpy(path, UNIX_TEMP_PATH);
#endif
	return path;
}

