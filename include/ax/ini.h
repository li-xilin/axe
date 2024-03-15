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

#ifndef AX_INI_H
#define AX_INI_H

#include "uchar.h"
#include "io.h"
#include <stdbool.h>

#define AX_INI_EBADNAME 1
#define AX_INI_ETOOLONG 2
#define AX_INI_ESYNTAX 3

#ifndef AX_INI_DEFINED
#define AX_INI_DEFINED
typedef struct ax_ini_st ax_ini;
#endif

typedef int ax_ini_parse_error_f(unsigned lineno, unsigned error, void *args);

ax_ini *ax_ini_create();

ax_ini *ax_ini_load(FILE *fp, ax_ini_parse_error_f *error_cb, void *args);

void ax_ini_dump(const ax_ini * d, FILE * out);

void ax_ini_free(ax_ini * d);

const ax_uchar *ax_ini_get(const ax_ini *d, const ax_uchar *sec_name, const ax_uchar *key);

ax_uchar *ax_ini_path_get(const ax_ini *d, const ax_uchar *path);

int ax_ini_set(ax_ini *d, const ax_uchar *sec_name, const ax_uchar *key, const ax_uchar *val);

int ax_ini_push_sec(ax_ini *d, const ax_uchar *sec_name);

int ax_ini_push_opt(ax_ini *d, const ax_uchar *key, const ax_uchar *val);

void ax_ini_unset(ax_ini *d, const ax_uchar *sec_name, const ax_uchar *key);

bool ax_ini_check_name(ax_uchar *name);

const ax_uchar *ax_ini_strerror(int errcode);

int ax_ini_get_bool(const ax_ini *d, const ax_uchar *seckey, bool dft_value);

int ax_ini_get_int(const ax_ini *d, const ax_uchar *seckey, int dft_value);

ax_uchar *ax_ini_get_str(const ax_ini *d, const ax_uchar *seckey, ax_uchar *dft_value);

#endif
