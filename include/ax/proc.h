/*
 * Copyright (c) 2023 Li Xilin <lixilin@gmx.com>
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

#ifndef AX_PROC_H
#define AX_PROC_H

#include "uchar.h"
#include "types.h"
#include <stdint.h>
#include <stdio.h>

#ifndef AX_PROC_DEFINED
#define AX_PROC_DEFINED
typedef struct ax_proc_st ax_proc;
#endif

ax_proc *ax_proc_open(const ax_uchar* fname);

FILE *ax_proc_stdin(const ax_proc *proc);

FILE *ax_proc_stdout(const ax_proc *proc);

FILE *ax_proc_stderr(const ax_proc *proc);

pid_t ax_proc_pid(const ax_proc *proc);

int ax_proc_kill(const ax_proc *proc);

int ax_proc_close(ax_proc *proc);

#endif
