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

#ifndef PATH_H
#define PATH_H

#include "uchar.h"

#include <ax/detect.h>

#include <stdio.h>
#include <limits.h>
#include <stdbool.h>

#ifdef AX_OS_WIN

#include <windef.h>
#define AX_PATH_SEP_CHAR L'\\'
#define AX_PATH_SEP ax_u("\\")
#define AX_PATH_MAX MAX_PATH + 1
#define AX_FILENAME_MAX 260

#else

#define AX_PATH_SEP_CHAR L'/'
#define AX_PATH_SEP ax_u("/")
#define AX_PATH_MAX PATH_MAX
#define AX_FILENAME_MAX NAME_MAX

#endif

void ax_path_empty(ax_uchar *path);

ax_uchar *ax_path_fixsep(ax_uchar *path);

ax_uchar *ax_path_trim(ax_uchar *path);

bool ax_path_is_absolute(const ax_uchar *path);

ax_uchar *ax_path_push(ax_uchar *path, size_t size, const ax_uchar *name);

const ax_uchar *ax_path_pop(ax_uchar *path);

ax_uchar *ax_path_join(ax_uchar *path, size_t size, ...);

ax_uchar *ax_path_normalize(ax_uchar *path);

ax_uchar *ax_path_realize(const ax_uchar *path, ax_uchar *resolved_path, size_t size);

const ax_uchar *ax_path_basename(const ax_uchar *path);

const ax_uchar *ax_path_extname(const ax_uchar *path);

ax_uchar *ax_path_getcwd(ax_uchar *path, size_t size);

int ax_path_setcwd(const ax_uchar *path);

ax_uchar *ax_path_homedir(ax_uchar *path, size_t size);

ax_uchar *ax_path_tmpdir(ax_uchar *path, size_t size);

#endif

