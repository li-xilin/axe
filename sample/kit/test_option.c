#include "ax/option.h"
#include "ax/io.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define OPT_MSG_INVALID "invalid option"
#define OPT_MSG_MISSING "option requires an argument"

static void print_argv(ax_uchar **argv)
{
	while (*argv) {
		ax_printf(ax_u("%" AX_PRIus " ") , *argv++);
	}
	printf("\n");
}

static void try_optparse(ax_uchar **argv)
{
	int opt;
	ax_uchar *arg;
	struct ax_option_st options;

	print_argv(argv);
	ax_option_init(&options, argv);
	while ((opt = ax_option_parse(&options, ax_u("abc:d::"))) != -1) {
		if (opt == '?') {
			printf("%s: %s\n", argv[0], options.errmsg);
		}
		printf("%c (%d) = '%s'\n", opt, options.optind, options.optarg);
	}
	ax_printf(ax_u("optind = %d\n"), options.optind);
	while ((arg = ax_option_arg(&options))) {
		printf("argument: %s\n", arg);
	}
}

static void try_optparse_long(ax_uchar **argv)
{
	ax_uchar *arg;
	int opt, longindex;
	struct ax_option_st options;
	struct ax_option_long_st longopts[] = {
		{ax_u("amend"), 'a', AX_OPT_NONE},
		{ax_u("brief"), 'b', AX_OPT_NONE},
		{ax_u("color"), 'c', AX_OPT_REQUIRED},
		{ax_u("delay"), 'd', AX_OPT_OPTIONAL},
		{ax_u("erase"), 256, AX_OPT_REQUIRED},
		{0, 0, 0}
	};

	print_argv(argv);
	ax_option_init(&options, argv);
	while ((opt = ax_option_parse_long(&options, longopts, &longindex)) != -1) {
		ax_uchar buf[2] = {0, 0};
		if (opt == '?') {
			printf("%s: %s\n", argv[0], options.errmsg);
		}
		buf[0] = opt;
		ax_printf(ax_u(AX_US(-6s) "(%d, %d) = '" AX_US() "'\n"),
				opt < 127 ? buf : longopts[longindex].longname,
				options.optind, longindex, options.optarg);
	}
	printf("optind = %d\n", options.optind);
	while ((arg = ax_option_arg(&options))) {
		printf("argument: %s\n", arg);
	}
}

static int manual_test(int argc, ax_uchar **argv)
{
	size_t size = (argc + 1) * sizeof(*argv);
	ax_uchar **argv_copy = malloc(size);

	memcpy(argv_copy, argv, size);
	printf("\nOPTPARSE\n");
	try_optparse(argv_copy);

	printf("\nOPTPARSE LONG\n");
	try_optparse_long(argv);
	return 0;
}

static int testsuite(void)
{
	struct config {
		ax_uchar amend;
		ax_uchar brief;
		ax_uchar *color;
		int delay;
		int erase;
	};
	struct {
		ax_uchar *argv[8];
		struct config conf;
		ax_uchar *args[8];
		ax_uchar *err;
	} t[] = {
		{
			{ax_u(""), ax_u("--"), ax_u("foobar"), 0},
			{0, 0, 0, 0, 0},
			{ax_u("foobar"), 0},
			0
		},
		{
			{ax_u(""), ax_u("-a"), ax_u("-b"), ax_u("-c"), ax_u("-d"), ax_u("10"), ax_u("-e"), 0},
			{1, 1, ax_u(""), 10, 1},
			{0},
			0
		},
		{
			{
				ax_u(""),
				ax_u("--amend"),
				ax_u("--brief"),
				ax_u("--color"),
				ax_u("--delay"),
				ax_u("10"),
				ax_u("--erase"),
				0
			},
			{1, 1, ax_u(""), 10, 1},
			{0},
			0
		},
		{
			{ax_u(""), ax_u("-a"), ax_u("-b"), ax_u("-cred"), ax_u("-d"), ax_u("10"), ax_u("-e"), 0},
			{1, 1, ax_u("red"), 10, 1},
			{0},
			0
		},
		{
			{ax_u(""), ax_u("-abcblue"), ax_u("-d10"), ax_u("foobar"), 0},
			{1, 1, ax_u("blue"), 10, 0},
			{ax_u("foobar"), 0},
			0
		},
		{
			{ax_u(""), ax_u("--color=red"), ax_u("-d"), ax_u("10"), ax_u("--"), ax_u("foobar"), 0},
			{0, 0, ax_u("red"), 10, 0},
			{ax_u("foobar"), 0},
			0
		},
		{
			{ax_u(""), ax_u("-eeeeee"), 0},
			{0, 0, 0, 0, 6},
			{0},
			0
		},
		{
			{ax_u(""), ax_u("--delay"), 0},
			{0, 0, 0, 0, 0},
			{0},
			ax_u(OPT_MSG_MISSING)
		},
		{
			{ax_u(""), ax_u("--foo"), ax_u("bar"), 0},
			{0, 0, 0, 0, 0},
			{ax_u("--foo"), ax_u("bar"), 0},
			ax_u(OPT_MSG_INVALID)
		},
		{
			{ax_u(""), ax_u("-x"), 0},
			{0, 0, 0, 0, 0},
			{ax_u("-x"), 0},
			ax_u(OPT_MSG_INVALID)
		},
		{
			{ax_u(""), ax_u("-"), 0},
			{0, 0, 0, 0, 0},
			{ax_u("-"), 0},
			0
		},
		{
			{ax_u(""), ax_u("-e"), ax_u("foo"), ax_u("bar"), ax_u("baz"), ax_u("-a"), ax_u("quux"), 0},
			{1, 0, 0, 0, 1},
			{ax_u("foo"), ax_u("bar"), ax_u("baz"), ax_u("quux"), 0},
			0
		},
		{
			{ax_u(""), ax_u("foo"), ax_u("--delay"), ax_u("1234"), ax_u("bar"), ax_u("-cred"), 0},
			{0, 0, ax_u("red"), 1234, 0},
			{ax_u("foo"), ax_u("bar"), 0},
			0
		},
	};
	int ntests = sizeof(t) / sizeof(*t);
	int i, nfails = 0;
	struct ax_option_long_st longopts[] = {
		{ax_u("amend"), 'a', AX_OPT_NONE},
		{ax_u("brief"), 'b', AX_OPT_NONE},
		{ax_u("color"), 'c', AX_OPT_OPTIONAL},
		{ax_u("delay"), 'd', AX_OPT_REQUIRED},
		{ax_u("erase"), 'e', AX_OPT_NONE},
		{0, 0, 0}
	};

	for (i = 0; i < ntests; i++) {
		int j, opt, longindex;
		ax_uchar *arg, *err = 0;
		struct ax_option_st options;
		struct config conf = {0, 0, 0, 0, 0};

		ax_option_init(&options, t[i].argv);
		while ((opt = ax_option_parse_long(&options, longopts, &longindex)) != -1) {
			switch (opt) {
				case 'a': conf.amend = 1; break;
				case 'b': conf.brief = 1; break;
				case 'c': conf.color = options.optarg ? options.optarg : ax_u(""); break;
				case 'd': conf.delay = ax_ustrtoint(options.optarg); break;
				case 'e': conf.erase++; break;
				default: err = options.errmsg;
			}
		}

		if (conf.amend != t[i].conf.amend) {
			nfails++;
			printf("FAIL (%2d): expected amend %d, got %d\n",
					i, t[i].conf.amend, conf.amend);
		}

		if (conf.brief != t[i].conf.brief) {
			nfails++;
			printf("FAIL (%2d): expected brief %d, got %d\n",
					i, t[i].conf.brief, conf.brief);
		}

		if (t[i].conf.color) {
			if (!conf.color || ax_ustrcmp(conf.color, t[i].conf.color)) {
				nfails++;
				ax_printf(ax_u("FAIL (%2d): expected color %" AX_PRIus ", got %" AX_PRIus "\n"),
						i, t[i].conf.color, conf.color ? conf.color : ax_u("(nil)"));
			}
		} else {
			if (conf.color) {
				nfails++;
				printf("FAIL (%2d): expected no color, got %s\n",
						i, conf.color);
			}
		}

		if (conf.delay != t[i].conf.delay) {
			nfails++;
			printf("FAIL (%2d): expected delay %d, got %d\n",
					i, t[i].conf.delay, conf.delay);
		}

		if (conf.erase != t[i].conf.erase) {
			nfails++;
			printf("FAIL (%2d): expected erase %d, got %d\n",
					i, t[i].conf.erase, conf.erase);
		}

		if (t[i].err) {
			if (!err || ax_ustrncmp(err, t[i].err, ax_ustrlen(t[i].err))) {
				nfails++;
				ax_printf(ax_u("FAIL (%2d): expected error '%" AX_PRIus "', got %" AX_PRIus "\n"),
						i, t[i].err, err && err[0] ? err : ax_u("(nil)"));
			}

		} else {
			if (err) {
				nfails++;
				printf("FAIL (%2d): expected no error, got %s\n",
						i, err);
			}

			for (j = 0; t[i].args[j]; j++) {
				arg = ax_option_arg(&options);
				if (!arg || ax_ustrcmp(arg, t[i].args[j])) {
					nfails++;
					ax_printf(ax_u("FAIL (%2d): expected arg %s, got %" AX_PRIus "\n"),
							i, t[i].args[j], arg ? arg : ax_u("(nil)"));
				}
			}
			if ((arg = ax_option_arg(&options))) {
				nfails++;
				printf("FAIL (%2d): expected no more args, got %s\n",
						i, arg);
			}
		}
	}

	if (nfails == 0) {
		puts("All tests pass.");
	}
	return nfails != 0;
}

int ax_main(int argc, ax_uchar **argv)
{
	if (argc > 1) {
		return manual_test(argc, argv);
	} else {
		return testsuite();
	}
}
