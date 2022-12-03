#ifndef AX_OPTION_H
#define AX_OPTION_H

struct ax_option_st {
    char **argv;
    int permute;
    int optind;
    int optopt;
    char *optarg;
    char errmsg[64];
    int subopt;
};

enum ax_option_argtype {
    AX_OPT_NONE,
    AX_OPT_REQUIRED,
    AX_OPT_OPTIONAL
};

struct ax_option_long_st {
    const char *longname;
    int shortname;
    enum ax_option_argtype argtype;
};

/**
 * Initializes the parser state.
 */
void ax_option_init(struct ax_option_st *options, char **argv);

/**
 * Read the next option in the argv array.
 * @param optstring a getopt()-formatted option string.
 * @return the next option character, -1 for done, or '?' for error
 *
 * Just like getopt(), a character followed by no colons means no
 * argument. One colon means the option has a required argument. Two
 * colons means the option takes an optional argument.
 */
int ax_option_parse(struct ax_option_st *options, const char *optstring);

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
char *ax_option_arg(struct ax_option_st *options);

int ax_argv_from_buf(char *line, char *argv[], int argc_max);

#endif
