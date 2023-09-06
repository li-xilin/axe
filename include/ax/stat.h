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

#ifndef AX_STAT_H
#define AX_STAT_H

#include "uchar.h"
#include "types.h"

#define AX_S_IFMT     0170000

#define AX_S_IFSOCK   0140000   /* socket */
#define AX_S_IFLNK    0120000   /* symbolic link */
#define AX_S_IFREG    0100000   /* regular file */
#define AX_S_IFBLK    0060000   /* block device */
#define AX_S_IFDIR    0040000   /* directory */
#define AX_S_IFCHR    0020000   /* character device */
#define AX_S_IFIFO    0010000   /* FIFO */

struct ax_stat_st {
	ax_dev_t  st_dev;         /* ID of device containing file */
	ax_ino_t  st_ino;         /* Inode number */
	int       st_mode;        /* File type and mode */
	int       st_nlink;       /* Number of hard links */
	pid_t     st_uid;         /* User ID of owner */
	pid_t     st_gid;         /* Group ID of owner */
	uint64_t  st_size;        /* Total size, in bytes */

	time_t st_atim;  /* Time of last access */
	time_t st_mtim;  /* Time of last modification */
	time_t st_ctim;  /* Time of last status change */
};

typedef struct ax_stat_st ax_stat;

int ax_stat_get(const ax_uchar* path, ax_stat *stat_buf);

#endif
