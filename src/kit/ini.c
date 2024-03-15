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

#include "ax/ini.h"
#include "ax/link.h"
#include "ax/debug.h"

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#ifdef AX_OS_WIN
#define ufprintf fwprintf
#else
#define ufprintf fprintf
#endif

#define BASE_LEN 1024
#define MAX_LEN (BASE_LEN * 1024 * 1024) /* aligned with BASE_LEN, here is 1GB */ 

#define DEFAULT_SEC_NAME ax_u("__default")

struct section
{
	ax_link link;
	ax_link opt_list;
	ax_uchar *name;
	size_t hash;
};

struct option
{
	ax_link link;
	ax_uchar *key;
	ax_uchar *val;
	size_t hash;
};

struct ax_ini_st
{
	size_t size;
	ax_link sec_list;
};

ax_ini *ax_ini_create()
{
	ax_ini *d = malloc(sizeof *d);
	if (!d)
		return NULL;

	d->size = 0 ;
	ax_link_init(&d->sec_list);
	return d ;
}

void ax_ini_free(ax_ini * d)
{
	if (!d)
		return;

	while (!ax_link_is_empty(&d->sec_list)) {
		struct section *sec = ax_link_first_entry(&d->sec_list, struct section, link);
		ax_link_del(&sec->link);
		while (!ax_link_is_empty(&sec->opt_list)) {
			struct option *opt = ax_link_first_entry(&sec->opt_list, struct option, link);
			ax_link_del(&opt->link);
			free(opt->key);
			free(opt->val);
			free(opt);
		}
		free(sec->name);
		free(sec);
	}
	free(d);
}

static struct option *find_option(struct section *sec, const ax_uchar *key)
{
	size_t key_hash = ax_ustrihash(key);
	ax_link *cur_opt;
	ax_link_foreach(cur_opt, &sec->opt_list) {
		struct option *opt = ax_link_entry(cur_opt, struct option, link);
		if (key_hash != opt->hash)
			continue;
		if (ax_ustricmp(key, opt->key))
			continue;
		return opt;
	}
	return NULL;
}

static struct section *find_section(const ax_ini *d, const ax_uchar *sec_name)
{
	if (!sec_name)
		sec_name = DEFAULT_SEC_NAME;

	size_t sec_hash = ax_ustrihash(sec_name);

	ax_link *cur_sec;
	ax_link_foreach(cur_sec, &d->sec_list) {
		struct section *sec = ax_link_entry(cur_sec, struct section, link);
		if (sec_hash != sec->hash)
			continue;
		if (ax_ustricmp(sec_name, sec->name))
			continue;
		return sec;
	}
	return NULL;
}

static struct section *find_section_with_len(const ax_ini *d, const ax_uchar *sec_name, size_t sec_name_len)
{
	if (!sec_name)
		sec_name = DEFAULT_SEC_NAME;

	size_t sec_hash = ax_ustrnihash(sec_name, sec_name_len);

	ax_link *cur_sec;
	ax_link_foreach(cur_sec, &d->sec_list) {
		struct section *sec = ax_link_entry(cur_sec, struct section, link);
		if (sec_hash != sec->hash)
			continue;
		if (ax_ustrnicmp(sec_name, sec->name, sec_name_len))
			continue;
		return sec;
	}
	return NULL;
}

const ax_uchar *ax_ini_get(const ax_ini *d, const ax_uchar *sec_name, const ax_uchar *key)
{
	struct section *sec = find_section(d, sec_name);
	if (!sec)
		return NULL;
	struct option *opt = find_option(sec, key);
	if (!opt)
		return NULL;
	return opt->val;
}

static void free_section(struct section *sec)
{
	if (!sec)
		return;
	free(sec->name);
	free(sec);
}

static void free_option(struct option *opt)
{
	if (opt) {
		free(opt->key);
		free(opt->val);
		free(opt);
	}
}

static struct section *alloc_section(const ax_uchar *sec_name)
{
	struct section *sec = NULL;

	if (!(sec = calloc(1, sizeof *sec)))
		goto fail;
	sec->hash = ax_ustrihash(sec_name);
	if (!(sec->name = ax_ustrdup(sec_name)))
		goto fail;
	ax_link_init(&sec->opt_list);
	ax_link_init_empty(&sec->link);
	return sec;
fail:
	free_section(sec);
	return NULL;
}

struct option *alloc_option(const ax_uchar *key, const ax_uchar *val)
{
	struct option *opt = NULL;

	if (!(opt = calloc(1, sizeof *opt)))
		goto fail;
	opt->hash = ax_ustrihash(key);
	if (!(opt->key = ax_ustrdup(key)))
		goto fail;
	if (!(opt->val = ax_ustrdup(val)))
		goto fail;
	ax_link_init_empty(&opt->link);
	return opt;
fail:
	free_option(opt);
	return NULL;
}

int ax_ini_set(ax_ini *d, const ax_uchar *sec_name, const ax_uchar *key, const ax_uchar *val)
{
	int retval = -1;
	struct option *opt = NULL;
	struct section *sec = NULL;

	if (!(sec = find_section(d, sec_name))) {
		if (!(sec = alloc_section(sec_name)))
			goto out;
		if (!(opt = alloc_option(key, val))) {
			free_section(sec);
			goto out;
		}
		ax_link_add_back(&sec->link, &d->sec_list);
		ax_link_add_back(&opt->link, &sec->opt_list);
		d->size++;
	}
	else if (!(opt = find_option(sec, key))){
		opt = alloc_option(key, val);
		if (!sec)
			goto out;
		ax_link_add_back(&opt->link, &sec->opt_list);
		d->size++;
	}
	else {
		ax_uchar *val_dup = ax_ustrdup(val);
		if (!val_dup)
			goto out;

		free(opt->val);
		opt->val = val_dup;
	}

	retval = 0;
out:
	return retval;
}

int ax_ini_push_sec(ax_ini *d, const ax_uchar *sec_name)
{
	int retval = -1;
	struct section *sec = NULL;
	if ((sec = find_section(d, sec_name))) {
		errno = EALREADY;
		goto out;
	}
	if (!(sec = alloc_section(sec_name)))
		goto out;
	ax_link_add_back(&sec->link, &d->sec_list);
	retval = 0;
out:
	return retval;
}

int ax_ini_push_opt(ax_ini *d, const ax_uchar *key, const ax_uchar *val)
{
	int retval = -1;
	struct option *opt = NULL;
	struct section *sec = NULL;
	if (!(sec = ax_link_last_entry_or_null(&d->sec_list, struct section, link))) {
		if (!(sec = alloc_section(DEFAULT_SEC_NAME)))
			goto out;
		ax_link_add_back(&sec->link, &d->sec_list);
	}

	else if ((opt = find_option(sec, key))){
		errno = EALREADY;
		return -1;
	}

	if (!(opt = alloc_option(key, val)))
		goto out;

	ax_link_add_back(&opt->link, &sec->opt_list);
	d->size++;
	retval = 0;
out:
	return retval;
}

void ax_ini_unset(ax_ini *d, const ax_uchar *sec_name, const ax_uchar *key)
{
	struct section *sec = find_section(d, sec_name);
	if (!sec)
		return;
	struct option *opt = find_option(sec, key);
	if (!opt)
		return;

	ax_link_del(&opt->link);
	d->size--;

	if (!ax_link_is_empty(&sec->opt_list))
		return;

	ax_link_del(&sec->link);
	free_section(sec);
	return ;
}

void ax_ini_dump(const ax_ini *d, FILE *out)
{
	ax_link *cur_sec, *cur_opt;
	ax_link_foreach(cur_sec, &d->sec_list) {
		struct section *sec = ax_link_entry(cur_sec, struct section, link);
		if (&sec->link != d->sec_list.next || ax_ustricmp(sec->name, DEFAULT_SEC_NAME) != 0)
			ufprintf(out, ax_u("[%") AX_PRIus ax_u("]\n"), sec->name);
		ax_link_foreach(cur_opt, &sec->opt_list) {
			struct option *opt = ax_link_entry(cur_opt, struct option, link);
			ufprintf(out, ax_u("%") AX_PRIus ax_u(" = %") AX_PRIus ax_u("\n"), opt->key, opt->val);
		}
	}
}

static ax_uchar *ustrstrip(ax_uchar *s)
{
	ax_uchar *last = NULL ;
	last = s + ax_ustrlen(s);
	while (isspace((int)*s) && *s)
		s++;
	while (last > s) {
		if (!isspace((int)*(last-1)))
			break ;
		last--;
	}
	*last = ax_u('\0');
	return s;
}

bool ax_ini_check_name(ax_uchar *name)
{
	if (name[0] == ax_u('\0'))
		return false;
	if (!isalpha(name[0]) && name[0] != ax_u('_'))
		return false;
	for (int i = 1; name[i]; i++) {
		if (!isalpha(name[i]) && !isdigit(name[i]) && name[0] != ax_u('_'))
			return false;
	}
	return true;
}

static int parse_line(ax_uchar *line_buf, ax_ini *d)
{
	int val_off = -1;
	// int comment_off = -1;

	ax_uchar *line = ustrstrip(line_buf);

	/* split string by specify token char */
	for (int i = 0; line[i]; i++) {
		if (line[i] == ax_u(';') || line[i] == ax_u('#')) {
			line[i] = ax_u('\0');
			// comment_off = i + 1;
			break;
		}

		if (val_off == -1 && line[i] == ax_u('=')) {
			line[i] = ax_u('\0');
			val_off = i + 1;
		}
	}

	ax_uchar *name = ustrstrip(line);
	if (val_off == -1) {
		if (name[0] == ax_u('\0'))
			return 0;

		if (name[0] != ax_u('['))
			return AX_INI_ESYNTAX;

		size_t len = ax_ustrlen(line);
		if (len == 0 || name[len - 1] != ax_u(']'))
			return AX_INI_EBADNAME;

		name[len - 1] = ax_u('\0');
		ax_uchar *sec_name = name + 1;
		if (!ax_ini_check_name(sec_name))
			return AX_INI_EBADNAME;

		if (ax_ini_push_sec(d, sec_name))
			return -1;
	}
	else {
		if (!ax_ini_check_name(name))
			return AX_INI_EBADNAME;

		ax_uchar *val = ustrstrip(line + val_off);
		if (ax_ini_push_opt(d, name, val))
			return -1;
	}
	return 0;
}

ax_ini *ax_ini_load(FILE *fp, ax_ini_parse_error_f *error_cb, void *args)
{
	ax_ini *d = NULL;
	ax_uchar *line_buf = NULL;
	size_t buf_offset = 0, buf_len = BASE_LEN, lineno = 1;
	char line[BASE_LEN];

	if (!(line_buf = malloc(sizeof(ax_uchar) * buf_len * 2)))
		goto fail;

	if (!(d = ax_ini_create()))
		goto fail;

	while (fgets(line, sizeof line, fp)) {
		int line_len = strlen(line);
		if (line_len == 0)
			continue;

		int ustr_len = ax_ustr_from_utf8(line_buf + buf_offset, buf_len - buf_offset, line);
		buf_offset += ustr_len;

		if (line_buf[buf_offset - 1] != ax_u('\n')) {
			/* incomplete text for a line */
			if (buf_len - buf_offset >= sizeof line)
				continue;

			buf_len *= 2;

			if (buf_len >= MAX_LEN) {
				/* max size limit exceed*/
				int ch;
				if (error_cb(lineno, AX_INI_ETOOLONG, args))
					goto out;
				/* consume the remaining chars in current line */
				while (((ch = fgetc(fp)) != ax_u('\n') && ch != EOF));
				buf_offset = 0;
			}
			else {
				ax_uchar *new_buf = realloc(line_buf, buf_len * sizeof(ax_uchar));
				if (!new_buf)
					goto fail;
				line_buf = new_buf;
			}
		}
		else {
			line_buf[buf_offset - 1] = ax_u('\0');
			buf_offset = 0;
			lineno++;

			int err = parse_line(line_buf, d);
			if (err == -1)
				goto fail;
			if (err > 0 && error_cb(lineno, err, args))
				goto out;
		}
	}
	if (!feof(fp)) {
		errno = EIO;
		goto fail;
	}
	goto out;
fail:
	ax_ini_free(d);
	d = NULL;
out:
	free(line_buf);
	return d;
}

const ax_uchar *ax_ini_strerror(int errcode)
{
	switch (errcode) {
		case AX_INI_EBADNAME: return ax_u("Invalid key name");
		case AX_INI_ETOOLONG: return ax_u("Too long for single line");
		case AX_INI_ESYNTAX: return ax_u("Syntax error");
		default: return ax_u("");
	}
}

static int section_name_len(const ax_uchar *path)
{
	for (int i = 0; path[i]; i++)
		if (path[i] == ax_u(':'))
			return i;
	return -1;
}

ax_uchar *ax_ini_path_get(const ax_ini *d, const ax_uchar *path)
{
	int sec_len = section_name_len(path);
	ax_assert(sec_len >= 0, "no colon(:) found in field path");
	struct section *sec = find_section_with_len(d, path, sec_len);
	if (!sec)
		return NULL;
	struct option *opt = find_option(sec, path + sec_len + 1);
	if (!opt)
		return NULL;
	return opt->val;
}

int ax_ini_get_bool(const ax_ini *d, const ax_uchar *path, bool dft_value)
{
	ax_uchar *value = ax_ini_path_get(d, path);
	if (!value)
		return dft_value;

	if (ax_ustricmp(value, ax_u("true")) == 0)
		return 1;
	if (ax_ustricmp(value, ax_u("false")) == 0)
		return 0;

	if (ax_ustricmp(value, ax_u("yes")) == 0)
		return 1;
	if (ax_ustricmp(value, ax_u("no")) == 0)
		return 0;

	if (ax_ustricmp(value, ax_u("y")) == 0)
		return 1;
	if (ax_ustricmp(value, ax_u("n")) == 0)
		return 0;

	if (ax_ustricmp(value, ax_u("1")) == 0)
		return 1;
	if (ax_ustricmp(value, ax_u("0")) == 0)
		return 0;

	return -1;
}

int ax_ini_get_int(const ax_ini *d, const ax_uchar *path, int dft_value)
{

	ax_uchar *value = ax_ini_path_get(d, path);
	if (!value)
		return dft_value;

	int num;
	if (ax_usscanf(value, ax_u("%d"), &num) != 1)
		return dft_value;
	return num;
}

ax_uchar *ax_ini_get_str(const ax_ini *d, const ax_uchar *path, ax_uchar *dft_value)
{
	ax_uchar *value = ax_ini_path_get(d, path);
	if (!value)
		return dft_value;
	return value;
}

