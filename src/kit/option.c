/*
 * Copyright (c) 2022-2023 Li Xilin <lixilin@gmx.com>
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

#include "ax/option.h"
#include "ax/mem.h"

#define OPT_MSG_INVALID ax_u("invalid option")
#define OPT_MSG_MISSING ax_u("option requires an argument")
#define OPT_MSG_TOOMANY ax_u("option takes no arguments")

static int option_error(struct ax_option_st *options, const ax_uchar *msg, const ax_uchar *data)
{
	unsigned p = 0;
	const ax_uchar *sep = " -- '";
	while (*msg)
		options->errmsg[p++] = *msg++;
	while (*sep)
		options->errmsg[p++] = *sep++;
	while (p < sizeof(options->errmsg) / sizeof(ax_uchar) - 2 && *data)
		options->errmsg[p++] = *data++;
	options->errmsg[p++] = '\'';
	options->errmsg[p++] = '\0';
	return ax_u('?');
}

void ax_option_init(struct ax_option_st *options, ax_uchar **argv)
{
	options->argv = argv;
	options->permute = 1;
	options->optind = argv[0] != 0;
	options->subopt = 0;
	options->optarg = 0;
	options->errmsg[0] = '\0';
}

static int option_is_dashdash(const ax_uchar *arg)
{
	return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] == '\0';
}

static int option_is_shortopt(const ax_uchar *arg)
{
	return arg != 0 && arg[0] == '-' && arg[1] != '-' && arg[1] != '\0';
}

static int option_is_longopt(const ax_uchar *arg)
{
	return arg != 0 && arg[0] == '-' && arg[1] == '-' && arg[2] != '\0';
}

static void option_permute(struct ax_option_st *options, int index)
{
	ax_uchar *nonoption = options->argv[index];
	int i;
	for (i = index; i < options->optind - 1; i++)
		options->argv[i] = options->argv[i + 1];
	options->argv[options->optind - 1] = nonoption;
}

static int option_argtype(const ax_uchar *optstring, char c)
{
	int count = AX_OPT_NONE;
	if (c == ':')
		return -1;
	for (; *optstring && c != *optstring; optstring++);
	if (!*optstring)
		return -1;
	if (optstring[1] == ax_u(':'))
		count += optstring[2] == ax_u(':') ? 2 : 1;
	return count;
}

int ax_option_parse(struct ax_option_st *options, const ax_uchar *optstring)
{
	int type;
	ax_uchar *next;
	ax_uchar *option = options->argv[options->optind];
	options->errmsg[0] = '\0';
	options->optopt = 0;
	options->optarg = 0;
	if (option == 0) {
		return -1;
	} else if (option_is_dashdash(option)) {
		options->optind++; /* consume "--" */
		return -1;
	} else if (!option_is_shortopt(option)) {
		if (options->permute) {
			int index = options->optind++;
			int r = ax_option_parse(options, optstring);
			option_permute(options, index);
			options->optind--;
			return r;
		} else {
			return -1;
		}
	}
	option += options->subopt + 1;
	options->optopt = option[0];
	type = option_argtype(optstring, option[0]);
	next = options->argv[options->optind + 1];
	switch (type) {
		case -1:
			;
			ax_uchar str[2] = {0, 0};
			str[0] = option[0];
			options->optind++;
			return option_error(options, OPT_MSG_INVALID, str);
		case AX_OPT_NONE:
			 if (option[1]) {
				 options->subopt++;
			 } else {
				 options->subopt = 0;
				 options->optind++;
			 }
			 return option[0];
		case AX_OPT_REQUIRED:
			 options->subopt = 0;
			 options->optind++;
			 if (option[1]) {
				 options->optarg = option + 1;
			 } else if (next != 0) {
				 options->optarg = next;
				 options->optind++;
			 } else {
				 ax_uchar str[2] = {0, 0};
				 str[0] = option[0];
				 options->optarg = 0;
				 return option_error(options, OPT_MSG_MISSING, str);
			 }
			 return option[0];
		case AX_OPT_OPTIONAL:
			 options->subopt = 0;
			 options->optind++;
			 if (option[1])
				 options->optarg = option + 1;
			 else
				 options->optarg = 0;
			 return option[0];
	}
	return 0;
}

ax_uchar *ax_option_arg(struct ax_option_st *options)
{
	ax_uchar *option = options->argv[options->optind];
	options->subopt = 0;
	if (option != 0)
		options->optind++;
	return option;
}

static int option_longopts_end(const struct ax_option_long_st *longopts, int i)
{
	return !longopts[i].longname && !longopts[i].shortname;
}

static void option_from_long(const struct ax_option_long_st *longopts, ax_uchar *optstring)
{
	ax_uchar *p = optstring;
	int i;
	for (i = 0; !option_longopts_end(longopts, i); i++) {
		if (longopts[i].shortname && longopts[i].shortname < 127) {
			int a;
			*p++ = longopts[i].shortname;
			for (a = 0; a < (int)longopts[i].argtype; a++)
				*p++ = ax_u(':');
		}
	}
	*p = '\0';
}

/* Unlike strcmp(), handles options containing "=". */
static int option_longopts_match(const ax_uchar *longname, const ax_uchar *option)
{
	const ax_uchar *a = option, *n = longname;
	if (longname == 0)
		return 0;
	for (; *a && *n && *a != ax_u('='); a++, n++)
		if (*a != *n)
			return 0;
	return *n == ax_u('\0') && (*a == ax_u('\0') || *a == ax_u('='));
}

/* Return the part after "=", or NULL. */
static ax_uchar * option_longopts_arg(ax_uchar *option)
{
	for (; *option && *option != ax_u('='); option++);
	if (*option == ax_u('='))
		return option + 1;
	else
		return 0;
}

static int option_long_fallback(struct ax_option_st *options,
		const struct ax_option_long_st *longopts,
		int *longindex)
{
	int result;
	ax_uchar optstring[96 * 3 + 1]; /* 96 ASCII printable characters */
	option_from_long(longopts, optstring);
	result = ax_option_parse(options, optstring);
	if (longindex != 0) {
		*longindex = -1;
		if (result != -1) {
			int i;
			for (i = 0; !option_longopts_end(longopts, i); i++)
				if (longopts[i].shortname == options->optopt)
					*longindex = i;
		}
	}
	return result;
}

int ax_option_parse_long(struct ax_option_st *options,
		const struct ax_option_long_st *longopts, int *longindex)
{
	int i;
	ax_uchar *option = options->argv[options->optind];
	if (option == 0) {
		return -1;
	} else if (option_is_dashdash(option)) {
		options->optind++; /* consume "--" */
		return -1;
	} else if (option_is_shortopt(option)) {
		return option_long_fallback(options, longopts, longindex);
	} else if (!option_is_longopt(option)) {
		if (options->permute) {
			int index = options->optind++;
			int r = ax_option_parse_long(options, longopts, longindex);
			option_permute(options, index);
			options->optind--;
			return r;
		} else {
			return -1;
		}
	}

	/* Parse as long option. */
	options->errmsg[0] = '\0';
	options->optopt = 0;
	options->optarg = 0;
	option += 2; /* skip "--" */
	options->optind++;
	for (i = 0; !option_longopts_end(longopts, i); i++) {
		const ax_uchar *name = longopts[i].longname;
		if (option_longopts_match(name, option)) {
			ax_uchar *arg;
			if (longindex)
				*longindex = i;
			options->optopt = longopts[i].shortname;
			arg = option_longopts_arg(option);
			if (longopts[i].argtype == AX_OPT_NONE && arg != 0) {
				return option_error(options, OPT_MSG_TOOMANY, name);
			} if (arg != 0) {
				options->optarg = arg;
			} else if (longopts[i].argtype == AX_OPT_REQUIRED) {
				options->optarg = options->argv[options->optind];
				if (options->optarg == 0)
					return option_error(options, OPT_MSG_MISSING, name);
				else
					options->optind++;
			}
			return options->optopt;
		}
	}
	return option_error(options, OPT_MSG_INVALID, option);
}

