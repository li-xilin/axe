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

#ifndef AX_SYS_H
#define AX_SYS_H

#include "uchar.h"
#include "types.h"

#include <ax/detect.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>

int ax_sys_mkdir(const ax_uchar *path, int mode);

int ax_sys_unlink(const ax_uchar *path);

int ax_sys_rename(const ax_uchar *path, const ax_uchar *new_path);

int ax_sys_copy(const ax_uchar *path, const ax_uchar *new_path);

int ax_sys_link(const ax_uchar *path, const ax_uchar *link_path);

/* 'dir_link' parameter specifies if the symbolic link is for a directory in Windows. */
int ax_sys_symlink(const ax_uchar *path, const ax_uchar *link_path, bool dir_link);

const ax_uchar *ax_sys_getenv(const ax_uchar *name);

int ax_sys_setenv(const ax_uchar *name, const ax_uchar *value);

int ax_sys_utime(const ax_uchar *path, time_t atime, time_t mtime);

int ax_sys_nprocs(void);

#endif

