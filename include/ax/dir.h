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

#ifndef AX_DIR_H
#define AX_DIR_H

#include "types.h"
#include <stdint.h>

#ifndef _WIN32
#include <dirent.h>

#define  AX_DT_UNKNOWN DT_UNKNOWN
#define  AX_DT_FIFO DT_FIFO
#define  AX_DT_CHR  DT_CHR
#define  AX_DT_DIR  DT_DIR
#define  AX_DT_BLK  DT_BLK
#define  AX_DT_REG  DT_REG
#define  AX_DT_LNK  DT_LNK
#define  AX_DT_SOCK DT_SOCK
#define  AX_DT_WHT  DT_WHT

typedef struct dirent ax_dirent; 
typedef DIR ax_dir;

inline static ax_dir *ax_dir_open(const char *path)
{
	return opendir(path);
}

inline static ax_dirent *ax_dir_read(ax_dir *dir)
{
	return readdir(dir);
}

inline static void ax_dir_seek(ax_dir* dirp, long int offset)
{
	seekdir(dirp, offset);
}

inline static long int ax_dir_tell(ax_dir* dirp)
{
	return telldir(dirp);
}

inline static void ax_dir_close(ax_dir *dir)
{
	closedir(dir);
}

#else

#include <sys/types.h>

#define AX_DT_FIFO 1
#define AX_DT_CHR 2
#define AX_DT_DIR 4
#define AX_DT_BLK 6
#define AX_DT_REG 8
#define AX_DT_LNK 10
#define AX_DT_SOCK 12
#define AX_DT_WHT 14

typedef struct __ax_dirent_st {
	ax_ino_t       d_ino;       /* Inode number */
	off_t          d_off;       /* Not an offset; see below */
	unsigned short d_reclen;    /* Length of this record */
	unsigned char  d_type;      /* Type of file; not supported
				       by all filesystem types */
	wchar_t        d_name[260]; /* Null-terminated filename */
} ax_dirent;

typedef struct __ax_dir_st ax_dir;

ax_dir *ax_dir_open(const wchar_t *path);

ax_dirent *ax_dir_read(ax_dir *dir);

void ax_dir_seek(ax_dir* dirp, long int offset);

long int ax_dir_tell(ax_dir* dirp);

void ax_dir_close(ax_dir *dir);

#endif

#endif
