/*
 * linenoise.c -- guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
 *
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez at gmail dot com>
 * Copyright (c) 2010, Pieter Noordhuis <pcnoordhuis at gmail dot com>
 * Copyright (c) 2011, Steve Bennett <steveb at workware dot net dot au>
 * Copyright (c) 2023, Li Xilin <lixilin@gmx.com>
 *
 * ------------------------------------------------------------------------
 *
 * References:
 * - http://invisible-island.net/xterm/ctlseqs/ctlseqs.html
 * - http://www.3waylabs.com/nw/WWW/products/wizcon/vt220.html
 *
 * Bloat:
 * - Completion?
 *
 * Unix/termios
 * ------------
 * List of escape sequences used by this program, we do everything just
 * a few sequences. In order to be so cheap we may have some
 * flickering effect with some slow terminal, but the lesser sequences
 * the more compatible.
 *
 * EL (Erase Line)
 *    Sequence: ESC [ 0 K
 *    Effect: clear from cursor to end of line
 *
 * CUF (CUrsor Forward)
 *    Sequence: ESC [ n C
 *    Effect: moves cursor forward n chars
 *
 * CR (Carriage Return)
 *    Sequence: \r
 *    Effect: moves cursor to column 1
 *
 * The following are used to clear the screen: ESC [ H ESC [ 2 J
 * This is actually composed of two sequences:
 *
 * cursorhome
 *    Sequence: ESC [ H
 *    Effect: moves the cursor to upper left corner
 *
 * ED2 (Clear entire screen)
 *    Sequence: ESC [ 2 J
 *    Effect: clear the whole screen
 *
 * == For highlighting control characters, we also use the following two ==
 * SO (enter StandOut)
 *    Sequence: ESC [ 7 m
 *    Effect: Uses some standout mode such as reverse video
 *
 * SE (Standout End)
 *    Sequence: ESC [ 0 m
 *    Effect: Exit standout mode
 *
 * == Only used if TIOCGWINSZ fails ==
 * DSR/CPR (Report cursor position)
 *    Sequence: ESC [ 6 n
 *    Effect: reports current cursor position as ESC [ NNN ; MMM R
 *
 * == Only used in multiline mode ==
 * CUU (Cursor Up)
 *    Sequence: ESC [ n A
 *    Effect: moves cursor up n chars.
 *
 * CUD (Cursor Down)
 *    Sequence: ESC [ n B
 *    Effect: moves cursor down n chars.
 *
 * win32/console
* -------------
* If __MINGW32__ is defined, the win32 console API is used.
* This could probably be made to work for the msvc compiler too.
* This support based in part on work by Jon Griffiths.
*/

#include "ax/edit.h"
#include "ax/io.h"
#include "ax/unicode.h"
#include "ax/mem.h"
#include "ax/uchar.h"
#include "stringbuf.h"

#ifdef _WIN32 /* Windows platform, either MinGW or Visual Studio (MSVC) */
#include <windows.h>
#include <fcntl.h>
#define USE_WINCONSOLE
#ifdef __MINGW32__
#define HAVE_UNISTD_H
#endif
#else
#include <termios.h>
#include <sys/ioctl.h>
#include <poll.h>
#define USE_TERMIOS
#define HAVE_UNISTD_H
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/types.h>

#if defined(_WIN32) && !defined(__MINGW32__)
/* Microsoft headers don't like old POSIX names */
#define snprintf _snprintf
#endif

#define LINENOISE_DEFAULT_HISTORY_MAX_LEN 100

/* ctrl('A') -> 0x01 */
#define ctrl(C) ((C) - '@')
/* meta('a') ->  0xe1 */
#define meta(C) ((C) | 0x80)

/* Use -ve numbers here to co-exist with normal unicode chars */
enum {
	SPECIAL_NONE,
	/* don't use -1 here since that indicates error */
	SPECIAL_UP = -20,
	SPECIAL_DOWN = -21,
	SPECIAL_LEFT = -22,
	SPECIAL_RIGHT = -23,
	SPECIAL_DELETE = -24,
	SPECIAL_HOME = -25,
	SPECIAL_END = -26,
	SPECIAL_INSERT = -27,
	SPECIAL_PAGE_UP = -28,
	SPECIAL_PAGE_DOWN = -29,

	/* Some handy names for other special keycodes */
	CHAR_ESCAPE = 27,
	CHAR_DELETE = 127,
};

static int history_max_len = LINENOISE_DEFAULT_HISTORY_MAX_LEN;
static int history_len = 0;
static int history_index = 0;
static ax_uchar **history = NULL;

/* Structure to contain the status of the current (being edited) line */
struct current {
	stringbuf *buf; /* Current buffer. Always null terminated */
	int pos;    /* Cursor position, measured in chars */
	int cols;   /* Size of the window, in chars */
	int nrows;  /* How many rows are being used in multiline mode (>= 1) */
	int rpos;   /* The current row containing the cursor - multiline mode only */
	int colsright; /* refresh_line() cached cols for insert_char() optimisation */
	int colsleft;  /* refresh_line() cached cols for remove_char() optimisation */
	const ax_uchar *prompt;
	stringbuf *capture; /* capture buffer, or NULL for none. Always null terminated */
	stringbuf *output;  /* used only during refresh_line() - output accumulator */
#if defined(USE_TERMIOS)
	int fd;     /* Terminal fd */
#elif defined(USE_WINCONSOLE)
	HANDLE outh; /* Console output handle */
	HANDLE inh; /* Console input handle */
	int rows;   /* Screen rows */
	int x;      /* Current column during output */
	int y;      /* Current row */
#define UBUF_MAX_CHARS 132
	WORD ubuf[UBUF_MAX_CHARS + 1];  /* Accumulates utf16 output - one extra for final surrogate pairs */
	int ubuflen;      /* length used in ubuf */
	int ubufcols;     /* how many columns are represented by the chars in ubuf? */
#endif
};

static int fd_read(struct current *current);
static int get_window_size(struct current *current);
static void cursor_down(struct current *current, int n);
static void cursor_up(struct current *current, int n);
static void erase_eol(struct current *current);
static void refresh_line(struct current *current);
static void refresh_line_alt(struct current *current, const ax_uchar *prompt, const ax_uchar *buf, int cursor_pos);
static void set_cursor_pos(struct current *current, int x);
static void set_output_highlight(struct current *current, const int *props, int nprops);
static void set_current(struct current *current, const ax_uchar *str);

static int fd_isatty(struct current *current)
{
#ifdef USE_TERMIOS
	return isatty(current->fd);
#else
	(void)current;
	return 0;
#endif
}

void ax_edit_history_free(void) {
	if (history) {
		int j;

		for (j = 0; j < history_len; j++)
			free(history[j]);
		free(history);
		history = NULL;
		history_len = 0;
	}
}

typedef enum {
	EP_START,   /* looking for ESC */
	EP_ESC,     /* looking for [ */
	EP_DIGITS,  /* parsing digits */
	EP_PROPS,   /* parsing digits or semicolons */
	EP_END,     /* ok */
	EP_ERROR,   /* error */
} ep_state_t;

struct esc_parser {
	ep_state_t state;
	int props[5];   /* properties are stored here */
	int maxprops;   /* size of the props[] array */
	int numprops;   /* number of properties found */
	int termchar;   /* terminator char, or 0 for any alpha */
	int current;    /* current (partial) property value */
};

/**
 * Initialise the escape sequence parser at *parser.
 *
 * If termchar is 0 any alpha char terminates ok. Otherwise only the given
 * char terminates successfully.
 * Run the parser state machine with calls to parse_escape_sequence() for each char.
 */
static void init_parse_escape_seq(struct esc_parser *parser, int termchar)
{
	parser->state = EP_START;
	parser->maxprops = ax_nelems(parser->props);
	parser->numprops = 0;
	parser->current = 0;
	parser->termchar = termchar;
}

/**
 * Pass character 'ch' into the state machine to parse:
 *   'ESC' '[' <digits> (';' <digits>)* <termchar>
 *
 * The first character must be ESC.
 * Returns the current state. The state machine is done when it returns either EP_END
 * or EP_ERROR.
 *
 * On EP_END, the "property/attribute" values can be read from parser->props[]
 * of length parser->numprops.
 */
static int parse_escape_sequence(struct esc_parser *parser, int ch)
{
	switch (parser->state) {
		case EP_START:
			parser->state = (ch == '\x1b') ? EP_ESC : EP_ERROR;
			break;
		case EP_ESC:
			parser->state = (ch == '[') ? EP_DIGITS : EP_ERROR;
			break;
		case EP_PROPS:
			if (ch == ';') {
				parser->state = EP_DIGITS;
donedigits:
				if (parser->numprops + 1 < parser->maxprops) {
					parser->props[parser->numprops++] = parser->current;
					parser->current = 0;
				}
				break;
			}
			/* fall through */
		case EP_DIGITS:
			if (ch >= '0' && ch <= '9') {
				parser->current = parser->current * 10 + (ch - '0');
				parser->state = EP_PROPS;
				break;
			}
			/* must be terminator */
			if (parser->termchar != ch) {
				if (parser->termchar != 0 || !((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z'))) {
					parser->state = EP_ERROR;
					break;
				}
			}
			parser->state = EP_END;
			goto donedigits;
		case EP_END:
			parser->state = EP_ERROR;
			break;
		case EP_ERROR:
			break;
	}
	return parser->state;
}

#define DEBUG_REFRESHLINE

#ifdef DEBUG_REFRESHLINE
#define DRL(...) ax_fprintf(dfh, __VA_ARGS__)
static FILE *dfh;

static void DRL_CHAR(int ch)
{
	if (ch < ' ') {
		DRL(ax_u("^%c"), ch + ax_u('@'));
	}
	/* TODO
	else if (ch > 127) {
		DRL(ax_u("\\u%04x"), ch);
	}
	*/
	else {
		DRL(ax_u("%c"), ch);
	}
}
static void DRL_STR(const ax_uchar *str)
{
	while (*str) {
		uint32_t ch;
		int n = ax_ustr_to_ucode(str, &ch);
		str += n;
		DRL_CHAR(ch);
	}
}
#else
#define DRL(...)
#define DRL_CHAR(ch)
#define DRL_STR(str)
#endif

#if defined(USE_WINCONSOLE)
#include "edit-win32.h"
#endif

#if defined(USE_TERMIOS)
static void edit_at_exit(void);
static struct termios orig_termios; /* in order to restore at exit */
static int rawmode = 0; /* for atexit() function to check if restore is needed*/
static int atexit_registered = 0; /* register atexit just 1 time */

static const char *unsupported_term[] = {"dumb","cons25","emacs",NULL};

static int is_unsupported_term(void) {
	char *term = getenv("TERM");

	if (term) {
		int j;
		for (j = 0; unsupported_term[j]; j++) {
			if (strcmp(term, unsupported_term[j]) == 0) {
				return 1;
			}
		}
	}
	return 0;
}

static int enable_raw_mode(struct current *current) {
	struct termios raw;

	current->fd = STDIN_FILENO;
	current->cols = 0;

	if (!isatty(current->fd) || is_unsupported_term() ||
			tcgetattr(current->fd, &orig_termios) == -1) {
fatal:
		errno = ENOTTY;
		return -1;
	}

	if (!atexit_registered) {
		atexit(edit_at_exit);
		atexit_registered = 1;
	}

	raw = orig_termios;  /* modify the original mode */
	/* input modes: no break, no CR to NL, no parity check, no strip char,
	 * no start/stop output control. */
	raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
	/* output modes - actually, no need to disable post processing */
	/*raw.c_oflag &= ~(OPOST);*/
	/* control modes - set 8 bit chars */
	raw.c_cflag |= (CS8);
	/* local modes - choing off, canonical off, no extended functions,
	 * no signal chars (^Z,^C) */
	raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
	/* control chars - set return condition: min number of bytes and timer.
	 * We want read to return every single byte, without timeout. */
	raw.c_cc[VMIN] = 1; raw.c_cc[VTIME] = 0; /* 1 byte, no timer */

	/* put terminal in raw mode after flushing */
	if (tcsetattr(current->fd,TCSADRAIN,&raw) < 0) {
		goto fatal;
	}
	rawmode = 1;
	return 0;
}

static void disable_raw_mode(struct current *current) {
	/* Don't even check the return value as it's too late. */
	if (rawmode && tcsetattr(current->fd,TCSADRAIN,&orig_termios) != -1)
		rawmode = 0;
}

/* At exit we'll try to fix the terminal to the initial conditions. */
static void edit_at_exit(void) {
	if (rawmode) {
		tcsetattr(STDIN_FILENO, TCSADRAIN, &orig_termios);
	}
	ax_edit_history_free();
}

/* gcc/glibc insists that we care about the return code of write!
 * Clarification: This means that a void-cast like "(void) (EXPR)"
 * does not work.
 */
#define IGNORE_RC(EXPR) if (EXPR) {}

/**
 * Output bytes directly, or accumulate output (if current->output is set)
 */
static void output_chars(struct current *current, const ax_uchar *buf, int len)
{
	if (len < 0) {
		len = ax_ustrlen(buf);
	}
	if (current->output) {
		sb_append_len(current->output, buf, len);
	}
	else {
		IGNORE_RC(write(current->fd, buf, len));
	}
}

/* Like output_chars, but using printf-style formatting
*/
static void output_formated(struct current *current, const ax_uchar *format, ...)
{
	va_list args;
	ax_uchar buf[64];
	int n;
	va_start(args, format);
	n = vsnprintf(buf, ax_nelems(buf), format, args);
	/* This will never happen because we are sure to use output_formated() only for short sequences */
	assert(n < ax_nelems(buf));
	va_end(args);
	output_chars(current, buf, n);
}

static void cursor_to_left(struct current *current)
{
	output_chars(current, "\r", -1);
}

static void set_output_highlight(struct current *current, const int *props, int nprops)
{
	output_chars(current, "\x1b[", -1);
	while (nprops--) {
		output_formated(current, "%d%c", *props, (nprops == 0) ? 'm' : ';');
		props++;
	}
}

static void erase_eol(struct current *current)
{
	output_chars(current, "\x1b[0K", -1);
}

static void set_cursor_pos(struct current *current, int x)
{
	if (x == 0) {
		cursor_to_left(current);
	}
	else {
		output_formated(current, "\r\x1b[%dC", x);
	}
}

static void cursor_up(struct current *current, int n)
{
	if (n) {
		output_formated(current, "\x1b[%dA", n);
	}
}

static void cursor_down(struct current *current, int n)
{
	if (n) {
		output_formated(current, "\x1b[%dB", n);
	}
}

void ax_edit_clear_screen(void)
{
	IGNORE_RC(write(STDOUT_FILENO, "\x1b[H\x1b[2J", 7));
}

/**
 * Reads a char from 'fd', waiting at most 'timeout' milliseconds.
 *
 * A timeout of -1 means to wait forever.
 *
 * Returns -1 if no char is received within the time or an error occurs.
 */
static int fd_read_char(int fd, int timeout)
{
	struct pollfd p;
	unsigned char c;

	p.fd = fd;
	p.events = POLLIN;

	if (poll(&p, 1, timeout) == 0) {
		/* timeout */
		return -1;
	}
	if (read(fd, &c, 1) != 1) {
		return -1;
	}
	return c;
}

/**
 * Reads a complete utf-8 character
 * and returns the unicode value, or -1 on error.
 */
static int fd_read(struct current *current)
{
	ax_uchar buf[MAX_UTF8_LEN];
	int n, i;
	uint32_t c;

	if (read(current->fd, &buf[0], 1) != 1) {
		return -1;
	}
	n = ax_utf8_charlen(buf[0]);
	if (n < 1) {
		return -1;
	}
	for (i = 1; i < n; i++) {
		if (read(current->fd, &buf[i], 1) != 1) {
			return -1;
		}
	}
	/* decode and return the character */
	ax_ustr_to_ucode(buf, &c);
	return c;
}

/**
 * Stores the current cursor column in '*cols'.
 * Returns 1 if OK, or 0 if failed to determine cursor pos.
 */
static int query_cursor(struct current *current, int* cols)
{
	struct esc_parser parser;
	int ch;

	/* Should not be buffering this output, it needs to go immediately */
	assert(current->output == NULL);

	/* control sequence - report cursor location */
	output_chars(current, "\x1b[6n", -1);

	/* Parse the response: ESC [ rows ; cols R */
	init_parse_escape_seq(&parser, 'R');
	while ((ch = fd_read_char(current->fd, 100)) > 0) {
		switch (parse_escape_sequence(&parser, ch)) {
			default:
				continue;
			case EP_END:
				if (parser.numprops == 2 && parser.props[1] < 1000) {
					*cols = parser.props[1];
					return 1;
				}
				break;
			case EP_ERROR:
				break;
		}
		/* failed */
		break;
	}
	return 0;
}

/**
 * Updates current->cols with the current window size (width)
 */
static int get_window_size(struct current *current)
{
	struct winsize ws;

	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col != 0) {
		current->cols = ws.ws_col;
		return 0;
	}

	/* Failed to query the window size. Perhaps we are on a serial terminal.
	 * Try to query the width by sending the cursor as far to the right
	 * and reading back the cursor position.
	 * Note that this is only done once per call to linenoise rather than
	 * every time the line is refreshed for efficiency reasons.
	 *
	 * In more detail, we:
	 * (a) request current cursor position,
	 * (b) move cursor far right,
	 * (c) request cursor position again,
	 * (d) at last move back to the old position.
	 * This gives us the width without messing with the externally
	 * visible cursor position.
	 */

	if (current->cols == 0) {
		int here;

		/* If anything fails => default 80 */
		current->cols = 80;

		/* (a) */
		if (query_cursor (current, &here)) {
			/* (b) */
			set_cursor_pos(current, 999);

			/* (c). Note: If (a) succeeded, then (c) should as well.
			 * For paranoia we still check and have a fallback action
			 * for (d) in case of failure..
			 */
			if (query_cursor(current, &current->cols)) {
				/* (d) Reset the cursor back to the original location. */
				if (current->cols > here) {
					set_cursor_pos(current, here);
				}
			}
		}
	}

	return 0;
}

/**
 * If CHAR_ESCAPE was received, reads subsequent
 * chars to determine if this is a known special key.
 *
 * Returns SPECIAL_NONE if unrecognised, or -1 if EOF.
 *
 * If no additional char is received within a short time,
 * CHAR_ESCAPE is returned.
 */
static int check_special(int fd)
{
	int c = fd_read_char(fd, 50);
	int c2;

	if (c < 0) {
		return CHAR_ESCAPE;
	}
	else if (c >= 'a' && c <= 'z') {
		/* esc-a => meta-a */
		return meta(c);
	}

	c2 = fd_read_char(fd, 50);
	if (c2 < 0) {
		return c2;
	}
	if (c == '[' || c == 'O') {
		/* Potential arrow key */
		switch (c2) {
			case 'A':
				return SPECIAL_UP;
			case 'B':
				return SPECIAL_DOWN;
			case 'C':
				return SPECIAL_RIGHT;
			case 'D':
				return SPECIAL_LEFT;
			case 'F':
				return SPECIAL_END;
			case 'H':
				return SPECIAL_HOME;
		}
	}
	if (c == '[' && c2 >= '1' && c2 <= '8') {
		/* extended escape */
		c = fd_read_char(fd, 50);
		if (c == '~') {
			switch (c2) {
				case '2':
					return SPECIAL_INSERT;
				case '3':
					return SPECIAL_DELETE;
				case '5':
					return SPECIAL_PAGE_UP;
				case '6':
					return SPECIAL_PAGE_DOWN;
				case '7':
					return SPECIAL_HOME;
				case '8':
					return SPECIAL_END;
			}
		}
		while (c != -1 && c != '~') {
			/* .e.g \e[12~ or '\e[11;2~   discard the complete sequence */
			c = fd_read_char(fd, 50);
		}
	}

	return SPECIAL_NONE;
}
#endif

static void clear_output_hightlight(struct current *current)
{
	int nohighlight = 0;
	set_output_highlight(current, &nohighlight, 1);
}

static void output_control_char(struct current *current, ax_uchar ch)
{
	int reverse = 7;
	set_output_highlight(current, &reverse, 1);
	output_chars(current, ax_u("^"), 1);
	output_chars(current, &ch, 1);
	clear_output_hightlight(current);
}

/**
 * Returns the unicode character at the given offset,
 * or -1 if none.
 */
static int get_char(struct current *current, int pos)
{
	if (pos >= 0 && pos < sb_chars(current->buf)) {
		uint32_t c;
		int i = ax_ustr_index(sb_str(current->buf), pos);
		(void)ax_ustr_to_ucode(sb_str(current->buf) + i, &c);
		return c;
	}
	return -1;
}

static int char_display_width(int ch)
{
	if (ch < ' ') {
		/* control chars take two positions */
		return 2;
	}
	else {
		return ax_ucode_width(ch);
	}
}

#ifndef NO_COMPLETION
static ax_edit_completion_cb *completionCallback = NULL;
static void *completionUserdata = NULL;
static int showhints = 1;
static ax_edit_hints_cb *hints_cb = NULL;
static ax_edit_free_hints_cb *free_hints_cb = NULL;
static void *hints_arg = NULL;

static void beep(void) {
#ifdef USE_TERMIOS
	fprintf(stderr, "\x7");
	fflush(stderr);
#endif
}

static void freeCompletions(ax_edit_completions *lc) {
	size_t i;
	for (i = 0; i < lc->len; i++)
		free(lc->cvec[i]);
	free(lc->cvec);
}

static int completeLine(struct current *current) {
	ax_edit_completions lc = { 0, NULL };
	int c = 0;

	completionCallback(sb_str(current->buf),&lc,completionUserdata);
	if (lc.len == 0) {
		beep();
	} else {
		size_t stop = 0, i = 0;

		while(!stop) {
			/* Show completion or original buffer */
			if (i < lc.len) {
				int chars = ax_ustr_charcnt(lc.cvec[i], -1);
				refresh_line_alt(current, current->prompt, lc.cvec[i], chars);
			} else {
				refresh_line(current);
			}

			c = fd_read(current);
			if (c == -1) {
				break;
			}

			switch(c) {
				case '\t': /* tab */
					i = (i+1) % (lc.len+1);
					if (i == lc.len) beep();
					break;
				case CHAR_ESCAPE: /* escape */
					/* Re-show original buffer */
					if (i < lc.len) {
						refresh_line(current);
					}
					stop = 1;
					break;
				default:
					/* Update buffer and return */
					if (i < lc.len) {
						set_current(current,lc.cvec[i]);
					}
					stop = 1;
					break;
			}
		}
	}

	freeCompletions(&lc);
	return c; /* Return last read character */
}

/* Register a callback function to be called for tab-completion.
   Returns the prior callback so that the caller may (if needed)
   restore it when done. */
ax_edit_completion_cb * ax_edit_set_completion_cb(ax_edit_completion_cb *fn, void *arg) {
	ax_edit_completion_cb * old = completionCallback;
	completionCallback = fn;
	completionUserdata = arg;
	return old;
}

void ax_edit_add_completion(ax_edit_completions *lc, const ax_uchar *str) {
	lc->cvec = (ax_uchar **)realloc(lc->cvec, sizeof(ax_uchar *) * (lc->len+1));
	lc->cvec[lc->len++] = ax_ustrdup(str);
}

void ax_edit_set_hints_cb(ax_edit_hints_cb *callback, void *arg)
{
	hints_cb = callback;
	hints_arg = arg;
}

void ax_edit_set_free_hints_cb(ax_edit_free_hints_cb *callback)
{
	free_hints_cb = callback;
}

#endif


static const ax_uchar *reduce_single_buf(const ax_uchar *buf, int availcols, int *cursor_pos)
{
	/* We have availcols columns available.
	 * If necessary, strip chars off the front of buf until *cursor_pos
	 * fits within availcols
	 */
	int needcols = 0;
	int pos = 0;
	int new_cursor_pos = *cursor_pos;
	const ax_uchar *pt = buf;

	DRL(ax_u("reduce_single_buf: availcols=%d, cursor_pos=%d\n"), availcols, *cursor_pos);

	while (*pt) {
		uint32_t ch;
		int n = ax_ustr_to_ucode(pt, &ch);
		pt += n;

		needcols += char_display_width(ch);

		/* If we need too many cols, strip
		 * chars off the front of buf to make it fit.
		 * We keep 3 extra cols to the right of the cursor.
		 * 2 for possible wide chars, 1 for the last column that
		 * can't be used.
		 */
		while (needcols >= availcols - 3) {
			n = ax_ustr_to_ucode(buf, &ch);
			buf += n;
			needcols -= char_display_width(ch);
			DRL_CHAR(ch);

			/* and adjust the apparent cursor position */
			new_cursor_pos--;

			if (buf == pt) {
				/* can't remove more than this */
				break;
			}
		}

		if (pos++ == *cursor_pos) {
			break;
		}

	}
	DRL(ax_u("<snip>"));
	DRL_STR(buf);
	DRL(ax_u("\nafter reduce, needcols=%d, new_cursor_pos=%d\n"), needcols, new_cursor_pos);

	/* Done, now new_cursor_pos contains the adjusted cursor position
	 * and buf points to he adjusted start
	 */
	*cursor_pos = new_cursor_pos;
	return buf;
}

static int mlmode = 0;

void ax_edit_set_multi_line(int enableml)
{
	mlmode = enableml;
}

/* Helper of refreshSingleLine() and refreshMultiLine() to show hints
 * to the right of the prompt.
 * Returns 1 if a hint was shown, or 0 if not
 * If 'display' is 0, does no output. Just returns the appropriate return code.
 */
static int refresh_show_hints(struct current *current, const ax_uchar *buf, int availcols, int display)
{
	int rc = 0;
	if (showhints && hints_cb && availcols > 0) {
		int bold = 0;
		int color = -1;
		ax_uchar *hint = hints_cb(buf, &color, &bold, hints_arg);
		if (hint) {
			rc = 1;
			if (display) {
				const ax_uchar *pt;
				if (bold == 1 && color == -1) color = 37;
				if (bold || color > 0) {
					int props[3] = { bold, color, 49 }; /* bold, color, fgnormal */
					set_output_highlight(current, props, 3);
				}
				DRL(ax_u("<hint bold=%d,color=%d>"), bold, color);
				pt = hint;
				while (*pt) {
					uint32_t ch;
					int n = ax_ustr_to_ucode(pt, &ch);
					int width = char_display_width(ch);

					if (width >= availcols) {
						DRL(ax_u("<hinteol>"));
						break;
					}
					DRL_CHAR(ch);

					availcols -= width;
					output_chars(current, pt, n);
					pt += n;
				}
				if (bold || color > 0) {
					clear_output_hightlight(current);
				}
				/* Call the function to free the hint returned. */
				if (free_hints_cb) free_hints_cb(hint, hints_arg);
			}
		}
	}
	return rc;
}

#ifdef USE_TERMIOS
static void refresh_start(struct current *current)
{
	/* We accumulate all output here */
	assert(current->output == NULL);
	current->output = sb_alloc();
}

static void refresh_end(struct current *current)
{
	/* Output everything at once */
	IGNORE_RC(write(current->fd, sb_str(current->output), sb_len(current->output)));
	sb_free(current->output);
	current->output = NULL;
}

static void refresh_start_chars(struct current *current)
{
	(void)current;
}

static void refresh_new_line(struct current *current)
{
	DRL(ax_u("<nl>"));
	output_chars(current, "\n", 1);
}

static void refresh_end_chars(struct current *current)
{
	(void)current;
}
#endif

static void refresh_line_alt(struct current *current, const ax_uchar *prompt, const ax_uchar *buf, int cursor_pos)
{
	int i;
	const ax_uchar *pt;
	int displaycol;
	int displayrow;
	int visible;
	int currentpos;
	int notecursor;
	int cursorcol = 0;
	int cursorrow = 0;
	int hint;
	struct esc_parser parser;

#ifdef DEBUG_REFRESHLINE
	dfh = fopen("linenoise.debuglog", "a");
#endif

	/* Should intercept SIGWINCH. For now, just get the size every time */
	get_window_size(current);

	refresh_start(current);

	DRL(ax_u("wincols=%d, cursor_pos=%d, nrows=%d, rpos=%d\n"), current->cols, cursor_pos, current->nrows, current->rpos);

	/* Here is the plan:
	 * (a) move the the bottom row, going down the appropriate number of lines
	 * (b) move to beginning of line and erase the current line
	 * (c) go up one line and do the same, until we have erased up to the first row
	 * (d) output the prompt, counting cols and rows, taking into account escape sequences
	 * (e) output the buffer, counting cols and rows
	 *   (e') when we hit the current pos, save the cursor position
	 * (f) move the cursor to the saved cursor position
	 * (g) save the current cursor row and number of rows
	 */

	/* (a) - The cursor is currently at row rpos */
	cursor_down(current, current->nrows - current->rpos - 1);
	DRL(ax_u("<cud=%d>"), current->nrows - current->rpos - 1);

	/* (b), (c) - Erase lines upwards until we get to the first row */
	for (i = 0; i < current->nrows; i++) {
		if (i) {
			DRL(ax_u("<cup>"));
			cursor_up(current, 1);
		}
		DRL(ax_u("<clearline>"));
		cursor_to_left(current);
		erase_eol(current);
	}
	DRL(ax_u("\n"));

	/* (d) First output the prompt. control sequences don't take up display space */
	pt = prompt;
	displaycol = 0; /* current display column */
	displayrow = 0; /* current display row */
	visible = 1;

	refresh_start_chars(current);

	while (*pt) {
		int width;
		uint32_t ch;
		int n = ax_ustr_to_ucode(pt, &ch);

		if (visible && ch == CHAR_ESCAPE) {
			/* The start of an escape sequence, so not visible */
			visible = 0;
			init_parse_escape_seq(&parser, 'm');
			DRL(ax_u("<esc-seq-start>"));
		}

		if (ch == '\n' || ch == '\r') {
			/* treat both CR and NL the same and force wrap */
			refresh_new_line(current);
			displaycol = 0;
			displayrow++;
		}
		else {
			width = visible * ax_ucode_width(ch);

			displaycol += width;
			if (displaycol >= current->cols) {
				/* need to wrap to the next line because of newline or if it doesn't fit
				 * XXX this is a problem in single line mode
				 */
				refresh_new_line(current);
				displaycol = width;
				displayrow++;
			}

			DRL_CHAR(ch);
#ifdef USE_WINCONSOLE
			if (visible) {
				output_chars(current, pt, n);
			}
#else
			output_chars(current, pt, n);
#endif
		}
		pt += n;

		if (!visible) {
			switch (parse_escape_sequence(&parser, ch)) {
				case EP_END:
					visible = 1;
					set_output_highlight(current, parser.props, parser.numprops);
					DRL(ax_u("<esc-seq-end,numprops=%d>"), parser.numprops);
					break;
				case EP_ERROR:
					DRL(ax_u("<esc-seq-err>"));
					visible = 1;
					break;
			}
		}

	}

	/* Now we are at the first line with all lines erased */
	DRL(ax_u("\nafter prompt: displaycol=%d, displayrow=%d\n"), displaycol, displayrow);


	/* (e) output the buffer, counting cols and rows */
	if (mlmode == 0) {
		/* In this mode we may need to trim chars from the start of the buffer until the
		 * cursor fits in the window.
		 */
		pt = reduce_single_buf(buf, current->cols - displaycol, &cursor_pos);
	}
	else {
		pt = buf;
	}

	currentpos = 0;
	notecursor = -1;

	while (*pt) {
		uint32_t ch;
		int n = ax_ustr_to_ucode(pt, &ch);
		int width = char_display_width(ch);

		if (currentpos == cursor_pos) {
			/* (e') wherever we output this character is where we want the cursor */
			notecursor = 1;
		}

		if (displaycol + width >= current->cols) {
			if (mlmode == 0) {
				/* In single line mode stop once we print as much as we can on one line */
				DRL(ax_u("<slmode>"));
				break;
			}
			/* need to wrap to the next line since it doesn't fit */
			refresh_new_line(current);
			displaycol = 0;
			displayrow++;
		}

		if (notecursor == 1) {
			/* (e') Save this position as the current cursor position */
			cursorcol = displaycol;
			cursorrow = displayrow;
			notecursor = 0;
			DRL(ax_u("<cursor>"));
		}

		displaycol += width;

		if (ch < ' ') {
			output_control_char(current, ch + '@');
		}
		else {
			output_chars(current, pt, n);
		}
		DRL_CHAR(ch);
		if (width != 1) {
			DRL(ax_u("<w=%d>"), width);
		}

		pt += n;
		currentpos++;
	}

	/* If we didn't see the cursor, it is at the current location */
	if (notecursor) {
		DRL(ax_u("<cursor>"));
		cursorcol = displaycol;
		cursorrow = displayrow;
	}

	DRL(ax_u("\nafter buf: displaycol=%d, displayrow=%d, cursorcol=%d, cursorrow=%d\n"), displaycol, displayrow, cursorcol, cursorrow);

	/* (f) show hints */
	hint = refresh_show_hints(current, buf, current->cols - displaycol, 1);

	/* Remember how many many cols are available for insert optimisation */
	if (prompt == current->prompt && hint == 0) {
		current->colsright = current->cols - displaycol;
		current->colsleft = displaycol;
	}
	else {
		/* Can't optimise */
		current->colsright = 0;
		current->colsleft = 0;
	}
	DRL(ax_u("\nafter hints: colsleft=%d, colsright=%d\n\n"), current->colsleft, current->colsright);

	refresh_end_chars(current);

	/* (g) move the cursor to the correct place */
	cursor_up(current, displayrow - cursorrow);
	set_cursor_pos(current, cursorcol);

	/* (h) Update the number of rows if larger, but never reduce this */
	if (displayrow >= current->nrows) {
		current->nrows = displayrow + 1;
	}
	/* And remember the row that the cursor is on */
	current->rpos = cursorrow;

	refresh_end(current);

#ifdef DEBUG_REFRESHLINE
	fclose(dfh);
#endif
}

static void refresh_line(struct current *current)
{
	refresh_line_alt(current, current->prompt, sb_str(current->buf), current->pos);
}

static void set_current(struct current *current, const ax_uchar *str)
{
	sb_clear(current->buf);
	sb_append(current->buf, str);
	current->pos = sb_chars(current->buf);
}

/**
 * Removes the char at 'pos'.
 *
 * Returns 1 if the line needs to be refreshed, 2 if not
 * and 0 if nothing was removed
 */
static int remove_char(struct current *current, int pos)
{
	if (pos >= 0 && pos < sb_chars(current->buf)) {
		int offset = ax_ustr_index(sb_str(current->buf), pos);
		int nchars = ax_ustr_index(sb_str(current->buf) + offset, 1);
		int rc = 1;

		/* Now we try to optimise in the simple but very common case that:
		 * - output_chars() can be used directly (not win32)
		 * - we are removing the char at EOL
		 * - the buffer is not empty
		 * - there are columns available to the left
		 * - the char being deleted is not a wide or utf-8 character
		 * - no hints are being shown
		 */
		if (current->output && current->pos == pos + 1 && current->pos == sb_chars(current->buf) && pos > 0) {
			/* Could implement ax_utf8_prev_len() but simplest just to not optimise this case */
			ax_uchar last = sb_str(current->buf)[offset];
			if (current->colsleft > 0 && (last & 0x80) == 0) {
				/* Have cols on the left and not a UTF-8 char or continuation */
				/* Yes, can optimise */
				current->colsleft--;
				current->colsright++;
				rc = 2;
			}
		}

		sb_delete(current->buf, offset, nchars);

		if (current->pos > pos) {
			current->pos--;
		}
		if (rc == 2) {
			if (refresh_show_hints(current, sb_str(current->buf), current->colsright, 0)) {
				/* A hint needs to be shown, so can't optimise after all */
				rc = 1;
			}
			else {
				/* optimised output */
				output_chars(current, ax_u("\b \b"), 3);
			}
		}
		return rc;
		return 1;
	}
	return 0;
}

/**
 * Insert 'ch' at position 'pos'
 *
 * Returns 1 if the line needs to be refreshed, 2 if not
 * and 0 if nothing was inserted (no room)
 */
static int insert_char(struct current *current, int pos, int ch)
{
	if (pos >= 0 && pos <= sb_chars(current->buf)) {
		ax_uchar buf[MAX_UTF8_LEN + 1];
		int offset = ax_ustr_index(sb_str(current->buf), pos);
		int n = ax_ucode_to_ustr(ch, buf);
		int rc = 1;

		/* null terminate since sb_insert() requires it */
		buf[n] = 0;

		/* Now we try to optimise in the simple but very common case that:
		 * - output_chars() can be used directly (not win32)
		 * - we are inserting at EOL
		 * - there are enough columns available
		 * - no hints are being shown
		 */
		if (current->output && pos == current->pos && pos == sb_chars(current->buf)) {
			int width = char_display_width(ch);
			if (current->colsright > width) {
				/* Yes, can optimise */
				current->colsright -= width;
				current->colsleft -= width;
				rc = 2;
			}
		}
		sb_insert(current->buf, offset, buf);
		if (current->pos >= pos) {
			current->pos++;
		}
		if (rc == 2) {
			if (refresh_show_hints(current, sb_str(current->buf), current->colsright, 0)) {
				/* A hint needs to be shown, so can't optimise after all */
				rc = 1;
			}
			else {
				/* optimised output */
				output_chars(current, buf, n);
			}
		}
		return rc;
	}
	return 0;
}

/**
 * Captures up to 'n' characters starting at 'pos' for the cut buffer.
 *
 * This replaces any existing characters in the cut buffer.
 */
static void capture_chars(struct current *current, int pos, int nchars)
{
	if (pos >= 0 && (pos + nchars - 1) < sb_chars(current->buf)) {
		int offset = ax_ustr_index(sb_str(current->buf), pos);
		int nbytes = ax_ustr_index(sb_str(current->buf) + offset, nchars);

		if (nbytes > 0) {
			if (current->capture) {
				sb_clear(current->capture);
			}
			else {
				current->capture = sb_alloc();
			}
			sb_append_len(current->capture, sb_str(current->buf) + offset, nbytes);
		}
	}
}

/**
 * Removes up to 'n' characters at cursor position 'pos'.
 *
 * Returns 0 if no chars were removed or non-zero otherwise.
 */
static int remove_chars(struct current *current, int pos, int n)
{
	int removed = 0;

	/* First save any chars which will be removed */
	capture_chars(current, pos, n);

	while (n-- && remove_char(current, pos)) {
		removed++;
	}
	return removed;
}
/**
 * Inserts the characters (string) 'chars' at the cursor position 'pos'.
 *
 * Returns 0 if no chars were inserted or non-zero otherwise.
 */
static int insert_chars(struct current *current, int pos, const ax_uchar *chars)
{
	int inserted = 0;

	while (*chars) {
		uint32_t ch;
		int n = ax_ustr_to_ucode(chars, &ch);
		if (insert_char(current, pos, ch) == 0) {
			break;
		}
		inserted++;
		pos++;
		chars += n;
	}
	return inserted;
}

static int skip_space_nonspace(struct current *current, int dir, int check_is_space)
{
	int moved = 0;
	int checkoffset = (dir < 0) ? -1 : 0;
	int limit = (dir < 0) ? 0 : sb_chars(current->buf);
	while (current->pos != limit && (get_char(current, current->pos + checkoffset) == ' ') == check_is_space) {
		current->pos += dir;
		moved++;
	}
	return moved;
}

static int skip_space(struct current *current, int dir)
{
	return skip_space_nonspace(current, dir, 1);
}

static int skip_nonspace(struct current *current, int dir)
{
	return skip_space_nonspace(current, dir, 0);
}

static void set_history_index(struct current *current, int new_index)
{
	if (history_len > 1) {
		/* Update the current history entry before to
		 * overwrite it with the next one. */
		free(history[history_len - 1 - history_index]);
		history[history_len - 1 - history_index] = ax_ustrdup(sb_str(current->buf));
		/* Show the new entry */
		history_index = new_index;
		if (history_index < 0) {
			history_index = 0;
		} else if (history_index >= history_len) {
			history_index = history_len - 1;
		} else {
			set_current(current, history[history_len - 1 - history_index]);
			refresh_line(current);
		}
	}
}

/**
 * Returns the keycode to process, or 0 if none.
 */
static int reverse_incremental_search(struct current *current)
{
	/* Display the reverse-i-search prompt and process chars */
	ax_uchar rbuf[50];
	ax_uchar rprompt[80];
	int rchars = 0;
	int rlen = 0;
	int searchpos = history_len - 1;
	int c;

	rbuf[0] = 0;
	while (1) {
		int n = 0;
		const ax_uchar *p = NULL;
		int skipsame = 0;
		int searchdir = -1;

		ax_usnprintf(rprompt, ax_nelems(rprompt), ax_u("(reverse-i-search)'%" AX_PRIus "': "), rbuf);
		refresh_line_alt(current, rprompt, sb_str(current->buf), current->pos);
		c = fd_read(current);
		if (c == ctrl('H') || c == CHAR_DELETE) {
			if (rchars) {
				int p_ind = ax_ustr_index(rbuf, --rchars);
				rbuf[p_ind] = 0;
				rlen = ax_ustrlen(rbuf);
			}
			continue;
		}
#ifdef USE_TERMIOS
		if (c == CHAR_ESCAPE) {
			c = check_special(current->fd);
		}
#endif
		if (c == ctrl('R')) {
			/* Search for the previous (earlier) match */
			if (searchpos > 0) {
				searchpos--;
			}
			skipsame = 1;
		}
		else if (c == ctrl('S')) {
			/* Search for the next (later) match */
			if (searchpos < history_len) {
				searchpos++;
			}
			searchdir = 1;
			skipsame = 1;
		}
		else if (c == ctrl('P') || c == SPECIAL_UP) {
			/* Exit Ctrl-R mode and go to the previous history line from the current search pos */
			set_history_index(current, history_len - searchpos);
			c = 0;
			break;
		}
		else if (c == ctrl('N') || c == SPECIAL_DOWN) {
			/* Exit Ctrl-R mode and go to the next history line from the current search pos */
			set_history_index(current, history_len - searchpos - 2);
			c = 0;
			break;
		}
		else if (c >= ' ' && c <= '~') {
			/* >= here to allow for null terminator */
			if (rlen >= (int)ax_nelems(rbuf) - MAX_UTF8_LEN) {
				continue;
			}

			n = ax_ucode_to_ustr(c, rbuf + rlen);
			rlen += n;
			rchars++;
			rbuf[rlen] = 0;

			/* Adding a new char resets the search location */
			searchpos = history_len - 1;
		}
		else {
			/* Exit from incremental search mode */
			break;
		}

		/* Now search through the history for a match */
		for (; searchpos >= 0 && searchpos < history_len; searchpos += searchdir) {
			p = ax_ustrstr(history[searchpos], rbuf);
			if (p) {
				/* Found a match */
				if (skipsame && ax_ustrcmp(history[searchpos], sb_str(current->buf)) == 0) {
					/* But it is identical, so skip it */
					continue;
				}
				/* Copy the matching line and set the cursor position */
				history_index = history_len - 1 - searchpos;
				set_current(current,history[searchpos]);
				current->pos = ax_ustr_charcnt(history[searchpos], p - history[searchpos]);
				break;
			}
		}
		if (!p && n) {
			/* No match, so don't add it */
			rchars--;
			rlen -= n;
			rbuf[rlen] = 0;
		}
	}
	if (c == ctrl('G') || c == ctrl('C')) {
		/* ctrl-g terminates the search with no effect */
		set_current(current, ax_u(""));
		history_index = 0;
		c = 0;
	}
	else if (c == ctrl('J')) {
		/* ctrl-j terminates the search leaving the buffer in place */
		history_index = 0;
		c = 0;
	}
	/* Go process the char normally */
	refresh_line(current);
	return c;
}

static int edit_line(struct current *current)
{
	history_index = 0;

	refresh_line(current);

	while(1) {
		int c = fd_read(current);

#ifndef NO_COMPLETION
		/* Only autocomplete when the callback is set. It returns < 0 when
		 * there was an error reading from fd. Otherwise it will return the
		 * character that should be handled next. */
		if (c == '\t' && current->pos == sb_chars(current->buf) && completionCallback != NULL) {
			c = completeLine(current);
		}
#endif
		if (c == ctrl('R')) {
			/* reverse incremental search will provide an alternative keycode or 0 for none */
			c = reverse_incremental_search(current);
			/* go on to process the returned char normally */
		}

#ifdef USE_TERMIOS
		if (c == CHAR_ESCAPE) {   /* escape sequence */
			c = check_special(current->fd);
		}
#endif
		if (c == -1) {
			/* Return on errors */
			return sb_len(current->buf);
		}

		switch(c) {
			case SPECIAL_NONE:
				break;
			case '\r':    /* enter/CR */
			case '\n':    /* LF */
				history_len--;
				free(history[history_len]);
				current->pos = sb_chars(current->buf);
				if (mlmode || hints_cb) {
					showhints = 0;
					refresh_line(current);
					showhints = 1;
				}
				return sb_len(current->buf);
			case ctrl('C'):     /* ctrl-c */
				errno = EAGAIN;
				return -1;
			case ctrl('Z'):     /* ctrl-z */
#ifdef SIGTSTP
				/* send ourselves SIGSUSP */
				disable_raw_mode(current);
				raise(SIGTSTP);
				/* and resume */
				enable_raw_mode(current);
				refresh_line(current);
#endif
				continue;
			case CHAR_DELETE:   /* backspace */
			case ctrl('H'):
				if (remove_char(current, current->pos - 1) == 1) {
					refresh_line(current);
				}
				break;
			case ctrl('D'):     /* ctrl-d */
				if (sb_len(current->buf) == 0) {
					/* Empty line, so EOF */
					history_len--;
					free(history[history_len]);
					return -1;
				}
				/* Otherwise fall through to delete char to right of cursor */
				/* fall-thru */
			case SPECIAL_DELETE:
				if (remove_char(current, current->pos) == 1) {
					refresh_line(current);
				}
				break;
			case SPECIAL_INSERT:
				/* Ignore. Expansion Hook.
				 * Future possibility: Toggle Insert/Overwrite Modes
				 */
				break;
			case meta('b'):    /* meta-b, move word left */
				if (skip_nonspace(current, -1)) {
					refresh_line(current);
				}
				else if (skip_space(current, -1)) {
					skip_nonspace(current, -1);
					refresh_line(current);
				}
				break;
			case meta('f'):    /* meta-f, move word right */
				if (skip_space(current, 1)) {
					refresh_line(current);
				}
				else if (skip_nonspace(current, 1)) {
					skip_space(current, 1);
					refresh_line(current);
				}
				break;
			case ctrl('W'):    /* ctrl-w, delete word at left. save deleted chars */
				/* eat any spaces on the left */
				{
					int pos = current->pos;
					while (pos > 0 && get_char(current, pos - 1) == ' ') {
						pos--;
					}

					/* now eat any non-spaces on the left */
					while (pos > 0 && get_char(current, pos - 1) != ' ') {
						pos--;
					}

					if (remove_chars(current, pos, current->pos - pos)) {
						refresh_line(current);
					}
				}
				break;
			case ctrl('T'):    /* ctrl-t */
				if (current->pos > 0 && current->pos <= sb_chars(current->buf)) {
					/* If cursor is at end, transpose the previous two chars */
					int fixer = (current->pos == sb_chars(current->buf));
					c = get_char(current, current->pos - fixer);
					remove_char(current, current->pos - fixer);
					insert_char(current, current->pos - 1, c);
					refresh_line(current);
				}
				break;
			case ctrl('V'):    /* ctrl-v */
				/* Insert the ^V first */
				if (insert_char(current, current->pos, c)) {
					refresh_line(current);
					/* Now wait for the next char. Can insert anything except \0 */
					c = fd_read(current);

					/* Remove the ^V first */
					remove_char(current, current->pos - 1);
					if (c > 0) {
						/* Insert the actual char, can't be error or null */
						insert_char(current, current->pos, c);
					}
					refresh_line(current);
				}
				break;
			case ctrl('B'):
			case SPECIAL_LEFT:
				if (current->pos > 0) {
					current->pos--;
					refresh_line(current);
				}
				break;
			case ctrl('F'):
			case SPECIAL_RIGHT:
				if (current->pos < sb_chars(current->buf)) {
					current->pos++;
					refresh_line(current);
				}
				break;
			case SPECIAL_PAGE_UP: /* move to start of history */
				set_history_index(current, history_len - 1);
				break;
			case SPECIAL_PAGE_DOWN: /* move to 0 == end of history, i.e. current */
				set_history_index(current, 0);
				break;
			case ctrl('P'):
			case SPECIAL_UP:
				set_history_index(current, history_index + 1);
				break;
			case ctrl('N'):
			case SPECIAL_DOWN:
				set_history_index(current, history_index - 1);
				break;
			case ctrl('A'): /* Ctrl+a, go to the start of the line */
			case SPECIAL_HOME:
				current->pos = 0;
				refresh_line(current);
				break;
			case ctrl('E'): /* ctrl+e, go to the end of the line */
			case SPECIAL_END:
				current->pos = sb_chars(current->buf);
				refresh_line(current);
				break;
			case ctrl('U'): /* Ctrl+u, delete to beginning of line, save deleted chars. */
				if (remove_chars(current, 0, current->pos)) {
					refresh_line(current);
				}
				break;
			case ctrl('K'): /* Ctrl+k, delete from current to end of line, save deleted chars. */
				if (remove_chars(current, current->pos, sb_chars(current->buf) - current->pos)) {
					refresh_line(current);
				}
				break;
			case ctrl('Y'): /* Ctrl+y, insert saved chars at current position */
				if (current->capture && insert_chars(current, current->pos, sb_str(current->capture))) {
					refresh_line(current);
				}
				break;
			case ctrl('L'): /* Ctrl+L, clear screen */
				ax_edit_clear_screen();
				/* Force recalc of window size for serial terminals */
				current->cols = 0;
				current->rpos = 0;
				refresh_line(current);
				break;
			default:
				if (c >= meta('a') && c <= meta('z')) {
					/* Don't insert meta chars that are not bound */
					break;
				}
				/* Only tab is allowed without ^V */
				if (c == '\t' || c >= ' ') {
					if (insert_char(current, current->pos, c) == 1) {
						refresh_line(current);
					}
				}
				break;
		}
	}
	return sb_len(current->buf);
}

int ax_edit_columns(void)
{
	struct current current;
	current.output = NULL;
	enable_raw_mode (&current);
	get_window_size (&current);
	disable_raw_mode (&current);
	return current.cols;
}

/**
 * Reads a line from the file handle (without the trailing NL or CRNL)
 * and returns it in a stringbuf.
 * Returns NULL if no characters are read before EOF or error.
 *
 * Note that the character count will *not* be correct for lines containing
 * utf8 sequences. Do not rely on the character count.
 */
static stringbuf *sb_getline(FILE *fh)
{
	stringbuf *sb = sb_alloc();
	int c;
	int n = 0;

	while ((c = getc(fh)) != EOF) {
		ax_uchar ch;
		n++;
		if (c == '\r') {
			/* CRLF -> LF */
			continue;
		}
		if (c == '\n' || c == '\r') {
			break;
		}
		ch = c;
		/* ignore the effect of character count for partial utf8 sequences */
		sb_append_len(sb, &ch, 1);
	}
	if (n == 0 || sb->data == NULL) {
		sb_free(sb);
		return NULL;
	}
	return sb;
}

ax_uchar *ax_edit_readline2(const ax_uchar *prompt, const ax_uchar *initial)
{
	int count;
	struct current current;
	stringbuf *sb;

	memset(&current, 0, sizeof(current));

	if (enable_raw_mode(&current) == -1) {
		ax_printf(ax_u("%" AX_PRIus), prompt);
		fflush(stdout);
		sb = sb_getline(stdin);
		if (sb && !fd_isatty(&current)) {
			ax_printf(ax_u("%" AX_PRIus "\n"), sb_str(sb));
			fflush(stdout);
		}
	}
	else {
		current.buf = sb_alloc();
		current.pos = 0;
		current.nrows = 1;
		current.prompt = prompt;

		/* The latest history entry is always our current buffer */
		ax_edit_history_add(initial);
		set_current(&current, initial);

		count = edit_line(&current);

		disable_raw_mode(&current);
		ax_printf(ax_u("\n"));

		sb_free(current.capture);
		if (count == -1) {
			sb_free(current.buf);
			return NULL;
		}
		sb = current.buf;
	}
	return sb ? sb_to_string(sb) : NULL;
}

ax_uchar *ax_edit_readline(const ax_uchar *prompt)
{
	return ax_edit_readline2(prompt, ax_u(""));
}

/* Using a circular buffer is smarter, but a bit more complex to handle. */
static int ax_edit_history_add_allocated(ax_uchar *line) {

	if (history_max_len == 0) {
notinserted:
		free(line);
		return 0;
	}
	if (history == NULL) {
		history = (ax_uchar **)calloc(sizeof(ax_uchar*), history_max_len);
	}

	/* do not insert duplicate lines into history */
	if (history_len > 0 && ax_ustrcmp(line, history[history_len - 1]) == 0) {
		goto notinserted;
	}

	if (history_len == history_max_len) {
		free(history[0]);
		memmove(history,history+1,sizeof(ax_uchar*)*(history_max_len-1));
		history_len--;
	}
	history[history_len] = line;
	history_len++;
	return 1;
}

int ax_edit_history_add(const ax_uchar *line) {
	return ax_edit_history_add_allocated(ax_ustrdup(line));
}

int ax_edit_history_maxlen(void) {
	return history_max_len;
}

int ax_edit_history_set_maxlen(int len) {
	ax_uchar **newHistory;

	if (len < 1) return 0;
	if (history) {
		int tocopy = history_len;

		newHistory = (ax_uchar **)calloc(sizeof(ax_uchar*), len);

		/* If we can't copy everything, free the elements we'll not use. */
		if (len < tocopy) {
			int j;

			for (j = 0; j < tocopy-len; j++) free(history[j]);
			tocopy = len;
		}
		memcpy(newHistory,history+(history_len-tocopy), sizeof(ax_uchar*) * tocopy);
		free(history);
		history = newHistory;
	}
	history_max_len = len;
	if (history_len > history_max_len)
		history_len = history_max_len;
	return 1;
}

/* Save the history in the specified file. On success 0 is returned
 * otherwise -1 is returned. */
int ax_edit_history_save(const ax_uchar *filename) {
	FILE *fp = ax_fopen(filename, ax_u("w"));
	int j;

	if (fp == NULL) return -1;
	for (j = 0; j < history_len; j++) {
		const ax_uchar *str = history[j];
		/* Need to encode backslash, nl and cr */
		while (*str) {
			if (*str == '\\')
				ax_fputs(ax_u("\\\\"), fp);
			else if (*str == '\n')
				ax_fputs(ax_u("\\n"), fp);
			else if (*str == '\r')
				ax_fputs(ax_u("\\r"), fp);
			else
				ax_fputc(*str, fp);
			str++;
		}
		ax_fputc(ax_u('\n'), fp);
	}

	fclose(fp);
	return 0;
}

/* Load the history from the specified file.
 *
 * If the file does not exist or can't be opened, no operation is performed
 * and -1 is returned.
 * Otherwise 0 is returned.
 */
int ax_edit_history_load(const ax_uchar *filename) {
	FILE *fp = ax_fopen(filename, ax_u("r"));
	stringbuf *sb;

	if (fp == NULL) return -1;

	while ((sb = sb_getline(fp)) != NULL) {
		/* Take the stringbuf and decode backslash escaped values */
		ax_uchar *buf = sb_to_string(sb);
		ax_uchar *dest = buf;
		const ax_uchar *src;

		for (src = buf; *src; src++) {
			ax_uchar ch = *src;

			if (ch == '\\') {
				src++;
				if (*src == 'n') {
					ch = '\n';
				}
				else if (*src == 'r') {
					ch = '\r';
				} else {
					ch = *src;
				}
			}
			*dest++ = ch;
		}
		*dest = 0;

		ax_edit_history_add_allocated(buf);
	}
	fclose(fp);
	return 0;
}

/* Provide access to the history buffer.
 *
 * If 'len' is not NULL, the length is stored in *len.
 */
ax_uchar **ax_edit_history(int *len) {
	if (len) {
		*len = history_len;
	}
	return history;
}
