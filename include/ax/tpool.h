/*
 * Copyright (c) 2022-2024 Li Xilin <lixilin@gmx.com>
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

#ifndef AX_TPOOL_H
#define AX_TPOOL_H

#include <stdbool.h>
#include <stddef.h>

#ifndef AX_TPOOL_DEFINED
#define AX_TPOOL_DEFINED
typedef struct ax_tpool_st ax_tpool;
#endif

typedef void ax_tpool_worker_f(void *arg);

ax_tpool *ax_tpool_create(size_t num);
void ax_tpool_destroy(ax_tpool *tpool);

int ax_tpool_add_work(ax_tpool *tpool, ax_tpool_worker_f *func, void *arg);
void ax_tpool_wait(ax_tpool *tpool);

#endif
