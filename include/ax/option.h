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

#ifndef AX_OPTION_H
#define AX_OPTION_H

#include "uchar.h"

#ifndef AX_OPTION_DEFINED
#define AX_OPTION_DEFINED
typedef struct ax_option_st ax_option;
#endif

#ifndef AX_OPTION_LONG_DEFINED
#define AX_OPTION_LONG_DEFINED
typedef struct ax_option_long_st ax_option_long;
#endif

struct ax_option_st {
    ax_uchar **argv;
    int permute;
    int optind;
    int optopt;
    ax_uchar *optarg;
    ax_uchar errmsg[64];
    int subopt;
};

enum ax_option_argtype {
    AX_OPT_NONE,
    AX_OPT_REQUIRED,
    AX_OPT_OPTIONAL
};

struct ax_option_long_st {
    const ax_uchar *longname;
    int shortname;
    enum ax_option_argtype argtype;
};

/**
 * Initializes the parser state.
 */
void ax_option_init(struct ax_option_st *options, ax_uchar **argv);

/**
 * Read the next option in the argv array.
 * @param optstring a getopt()-formatted option string.
 * @return the next option character, -1 for done, or '?' for error
 *
 * Just like getopt(), a character followed by no colons means no
 * argument. One colon means the option has a required argument. Two
 * colons means the option takes an optional argument.
 */
int ax_option_parse(struct ax_option_st *options, const ax_uchar *optstring);

/**
 * Handles GNU-style long options in addition to getopt() options.
 * This works a lot like GNU's getopt_long(). The last option in
 * longopts must be all zeros, marking the end of the array. The
 * longindex argument may be NULL.
 */
int ax_option_parse_long(struct ax_option_st *options,
                  const struct ax_option_long_st *longopts,
                  int *longindex);

/**
 * Used for stepping over non-option arguments.
 * @return the next non-option argument, or NULL for no more arguments
 *
 * Argument parsing can continue with ax_option_st() after using this
 * function. That would be used to parse the options for the
 * subcommand returned by ax_option_arg(). This function allows you to
 * ignore the value of optind.
 */
ax_uchar *ax_option_arg(struct ax_option_st *options);

#endif
