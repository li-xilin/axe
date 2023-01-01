// Copyright (c) 2019, JC Wang (wang_junchuan@163.com)

#include "ax/edit.h"

#ifdef _WIN32
	#include <io.h>
	#include <conio.h>
	#include <windows.h>
  #ifndef STDIN_FILENO
	#define STDIN_FILENO 			_fileno(stdin)
	#define STDOUT_FILENO 			_fileno(stdout)
  #endif
	#define isatty _isatty
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
	static int s_crossline_win = 1;
#else
	#include <unistd.h>
	#include <termios.h>
	#include <fcntl.h>
	#include <signal.h>
	#include <sys/ioctl.h>
	#include <sys/stat.h>
	static int s_crossline_win = 0;
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <stdint.h>

/*****************************************************************************/

// Default word delimiters for move and cut
#define CROSS_DFT_DELIMITER			" !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~"

#define CROSS_HISTORY_MAX_LINE		256		// Maximum history line number
#define CROSS_HISTORY_BUF_LEN		4096	// History line length
#define CROSS_HIS_MATCH_PAT_NUM		16		// History search pattern number

#define CROSS_COMPLET_MAX_LINE		1024	// Maximum completion word number
#define CROSS_COMPLET_WORD_LEN		64		// Completion word length
#define CROSS_COMPLET_HELP_LEN		256		// Completion word's help length
#define CROSS_COMPLET_HINT_LEN		128		// Completion syntax hints length

// Make control-characters readable
#define CTRL_KEY(key)				(key - 0x40)
// Build special key code for escape sequences
#define ALT_KEY(key)				(key + ((KEY_ESC+1)<<8))
#define ESC_KEY3(ch)				((KEY_ESC<<8) + ch)
#define ESC_KEY4(ch1,ch2)			((KEY_ESC<<8) + ((ch1)<<16) + ch2)
#define ESC_KEY6(ch1,ch2,ch3)		((KEY_ESC<<8) + ((ch1)<<16) + ((ch2)<<24) + ch3)
#define ESC_OKEY(ch)				((KEY_ESC<<8) + ('O'<<16) + ch)

/*****************************************************************************/

enum {
	KEY_TAB			= 9,	// Autocomplete.
	KEY_BACKSPACE	= 8,	// Delete character before cursor.
	KEY_ENTER		= 13,	// Accept line. (Linux)
	KEY_ENTER2		= 10,	// Accept line. (Windows)
	KEY_ESC			= 27,	// Escapce
	KEY_DEL2		= 127,  // It's treaded as Backspace is Linux
	KEY_DEBUG		= 30,	// Ctrl-^ Enter keyboard debug mode

#ifdef _WIN32 // Windows

	KEY_INSERT		= (KEY_ESC<<8) + 'R', // Paste last cut text.
	KEY_DEL			= (KEY_ESC<<8) + 'S', // Delete character under cursor.
	KEY_HOME		= (KEY_ESC<<8) + 'G', // Move cursor to start of line.
	KEY_END			= (KEY_ESC<<8) + 'O', // Move cursor to end of line.
	KEY_PGUP		= (KEY_ESC<<8) + 'I', // Move to first line in history.
	KEY_PGDN		= (KEY_ESC<<8) + 'Q', // Move to end of input history.
	KEY_UP			= (KEY_ESC<<8) + 'H', // Fetch previous line in history.
	KEY_DOWN		= (KEY_ESC<<8) + 'P', // Fetch next line in history.
	KEY_LEFT		= (KEY_ESC<<8) + 'K', // Move back a character.
	KEY_RIGHT		= (KEY_ESC<<8) + 'M', // Move forward a character.

	KEY_CTRL_DEL	= (KEY_ESC<<8) + 147, // Cut word following cursor.
	KEY_CTRL_HOME	= (KEY_ESC<<8) + 'w', // Cut from start of line to cursor.
	KEY_CTRL_END	= (KEY_ESC<<8) + 'u', // Cut from cursor to end of line.
	KEY_CTRL_UP		= (KEY_ESC<<8) + 141, // Uppercase current or following word.
	KEY_CTRL_DOWN	= (KEY_ESC<<8) + 145, // Lowercase current or following word.
	KEY_CTRL_LEFT	= (KEY_ESC<<8) + 's', // Move back a word.
	KEY_CTRL_RIGHT	= (KEY_ESC<<8) + 't', // Move forward a word.
	KEY_CTRL_BACKSPACE	= (KEY_ESC<<8) + 127, // Cut from start of line to cursor.

	KEY_ALT_DEL		= ALT_KEY(163),		// Cut word following cursor.
	KEY_ALT_HOME	= ALT_KEY(151), 	// Cut from start of line to cursor.
	KEY_ALT_END		= ALT_KEY(159), 	// Cut from cursor to end of line.
	KEY_ALT_UP		= ALT_KEY(152),		// Uppercase current or following word.
	KEY_ALT_DOWN	= ALT_KEY(160),		// Lowercase current or following word.
	KEY_ALT_LEFT	= ALT_KEY(155),		// Move back a word.
	KEY_ALT_RIGHT	= ALT_KEY(157),		// Move forward a word.
	KEY_ALT_BACKSPACE	= ALT_KEY(KEY_BACKSPACE), // Cut from start of line to cursor.

	KEY_F1			= (KEY_ESC<<8) + ';',	// Show help.
	KEY_F2			= (KEY_ESC<<8) + '<',	// Show history.
	KEY_F3			= (KEY_ESC<<8) + '=',	// Clear history (need confirm).
	KEY_F4			= (KEY_ESC<<8) + '>',	// Search history with current input.

#else // Linux

	KEY_INSERT		= ESC_KEY4('2','~'),	// vt100 Esc[2~: Paste last cut text.
	KEY_DEL			= ESC_KEY4('3','~'),	// vt100 Esc[3~: Delete character under cursor.
	KEY_HOME		= ESC_KEY4('1','~'),	// vt100 Esc[1~: Move cursor to start of line.
	KEY_END			= ESC_KEY4('4','~'),	// vt100 Esc[4~: Move cursor to end of line.
	KEY_PGUP		= ESC_KEY4('5','~'),	// vt100 Esc[5~: Move to first line in history.
	KEY_PGDN		= ESC_KEY4('6','~'),	// vt100 Esc[6~: Move to end of input history.
	KEY_UP			= ESC_KEY3('A'), 		//       Esc[A: Fetch previous line in history.
	KEY_DOWN		= ESC_KEY3('B'),		//       Esc[B: Fetch next line in history.
	KEY_LEFT		= ESC_KEY3('D'), 		//       Esc[D: Move back a character.
	KEY_RIGHT		= ESC_KEY3('C'), 		//       Esc[C: Move forward a character.
	KEY_HOME2		= ESC_KEY3('H'),		// xterm Esc[H: Move cursor to start of line.
	KEY_END2		= ESC_KEY3('F'),		// xterm Esc[F: Move cursor to end of line.

	KEY_CTRL_DEL	= ESC_KEY6('3','5','~'), // xterm Esc[3;5~: Cut word following cursor.
	KEY_CTRL_HOME	= ESC_KEY6('1','5','H'), // xterm Esc[1;5H: Cut from start of line to cursor.
	KEY_CTRL_END	= ESC_KEY6('1','5','F'), // xterm Esc[1;5F: Cut from cursor to end of line.
	KEY_CTRL_UP		= ESC_KEY6('1','5','A'), // xterm Esc[1;5A: Uppercase current or following word.
	KEY_CTRL_DOWN	= ESC_KEY6('1','5','B'), // xterm Esc[1;5B: Lowercase current or following word.
	KEY_CTRL_LEFT	= ESC_KEY6('1','5','D'), // xterm Esc[1;5D: Move back a word.
	KEY_CTRL_RIGHT	= ESC_KEY6('1','5','C'), // xterm Esc[1;5C: Move forward a word.
	KEY_CTRL_BACKSPACE	= 31, 				 // xterm Cut from start of line to cursor.
	KEY_CTRL_UP2	= ESC_OKEY('A'),		 // vt100 EscOA: Uppercase current or following word.
	KEY_CTRL_DOWN2	= ESC_OKEY('B'), 		 // vt100 EscOB: Lowercase current or following word.
	KEY_CTRL_LEFT2	= ESC_OKEY('D'),		 // vt100 EscOD: Move back a word.
	KEY_CTRL_RIGHT2	= ESC_OKEY('C'),		 // vt100 EscOC: Move forward a word.

	KEY_ALT_DEL		= ESC_KEY6('3','3','~'), // xterm Esc[3;3~: Cut word following cursor.
	KEY_ALT_HOME	= ESC_KEY6('1','3','H'), // xterm Esc[1;3H: Cut from start of line to cursor.
	KEY_ALT_END		= ESC_KEY6('1','3','F'), // xterm Esc[1;3F: Cut from cursor to end of line.
	KEY_ALT_UP		= ESC_KEY6('1','3','A'), // xterm Esc[1;3A: Uppercase current or following word.
	KEY_ALT_DOWN	= ESC_KEY6('1','3','B'), // xterm Esc[1;3B: Lowercase current or following word.
	KEY_ALT_LEFT	= ESC_KEY6('1','3','D'), // xterm Esc[1;3D: Move back a word.
	KEY_ALT_RIGHT	= ESC_KEY6('1','3','C'), // xterm Esc[1;3C: Move forward a word.
	KEY_ALT_BACKSPACE	= ALT_KEY(KEY_DEL2), // Cut from start of line to cursor.

	KEY_F1			= ESC_OKEY('P'),		 // 	  EscOP: Show help.
	KEY_F2			= ESC_OKEY('Q'),		 // 	  EscOQ: Show history.
	KEY_F3			= ESC_OKEY('R'),		 //       EscOP: Clear history (need confirm).
	KEY_F4			= ESC_OKEY('S'),		 //       EscOP: Search history with current input.

	KEY_F1_2		= ESC_KEY4('[', 'A'),	 // linux Esc[[A: Show help.
	KEY_F2_2		= ESC_KEY4('[', 'B'),	 // linux Esc[[B: Show history.
	KEY_F3_2		= ESC_KEY4('[', 'C'),	 // linux Esc[[C: Clear history (need confirm).
	KEY_F4_2		= ESC_KEY4('[', 'D'),	 // linux Esc[[D: Search history with current input.

#endif
};

/*****************************************************************************/

struct ax_edit_completions_st {
	int		num;
	char	word[CROSS_COMPLET_MAX_LINE][CROSS_COMPLET_WORD_LEN];
	char	help[CROSS_COMPLET_MAX_LINE][CROSS_COMPLET_HELP_LEN];
	char	hints[CROSS_COMPLET_HINT_LEN];
	ax_edit_color_e	color_word[CROSS_COMPLET_MAX_LINE];
	ax_edit_color_e	color_help[CROSS_COMPLET_MAX_LINE];
	ax_edit_color_e	color_hints;
};

static char		s_word_delimiter[64] = CROSS_DFT_DELIMITER;
static char 	s_history_buf[CROSS_HISTORY_MAX_LINE][CROSS_HISTORY_BUF_LEN];
static uint32_t s_history_id = 0; // Increase always, wrap until UINT_MAX
static char 	s_clip_buf[CROSS_HISTORY_BUF_LEN]; // Buf to store cut text
static ax_edit_completion_callback s_completion_callback = NULL;
static int		s_paging_print_line = 0; // For paging control
static int		s_got_resize 		= 0; // Window size changed
static ax_edit_color_e s_prompt_color = AX_EDIT_COLOR_DEFAULT;

static char* 	ax_edit_readline_edit (char *buf, int size, const char *prompt, int has_input, int in_his);
static int		ax_edit_history_dump (FILE *file, int print_id, char *patterns, int sel_id, int paging);

#define isdelim(ch)		(NULL != strchr(s_word_delimiter, ch))	// Check ch is word delimiter

// Debug macro.
#if 0
static FILE *s_crossline_debug_fp = NULL;
#define ax_edit_debug(...) \
	do { \
		if (NULL == s_crossline_debug_fp) { s_crossline_debug_fp = fopen("ax_edit_debug.txt", "a"); } \
		fprintf (s_crossline_debug_fp, __VA_ARGS__); \
		fflush (s_crossline_debug_fp); \
	} while (0)
#else
#define ax_edit_debug(...)
#endif

/*****************************************************************************/

static char* s_crossline_help[] = {
" Misc Commands",
" +-------------------------+--------------------------------------------------+",
" | F1                      |  Show edit shortcuts help.                       |",
" | Ctrl-^                  |  Enter keyboard debugging mode.                  |",
" +-------------------------+--------------------------------------------------+",
" Move Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-B, Left            |  Move back a character.                          |",
" | Ctrl-F, Right           |  Move forward a character.                       |",
" | Up, ESC+Up              |  Move cursor to up line. (For multiple lines)    |",
" |   Ctrl-Up, Alt-Up       |  (Ctrl-Up, Alt-Up only supports Windows/Xterm)   |",
" | Down, ESC+Down          |  Move cursor to down line. (For multiple lines)  |",
" |   Ctrl-Down,Alt-Down    |  (Ctrl-Down, Alt-Down only support Windows/Xterm)|",
" | Alt-B, ESC+Left,        |  Move back a word.                               |",
" |   Ctrl-Left, Alt-Left   |  (Ctrl-Left, Alt-Left only support Windows/Xterm)|",
" | Alt-F, ESC+Right,       |  Move forward a word.                            |",
" |   Ctrl-Right, Alt-Right | (Ctrl-Right,Alt-Right only support Windows/Xterm)|",
" | Ctrl-A, Home            |  Move cursor to start of line.                   |",
" | Ctrl-E, End             |  Move cursor to end of line.                     |",
" | Ctrl-L                  |  Clear screen and redisplay line.                |",
" +-------------------------+--------------------------------------------------+",
" Edit Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-H, Backspace       |  Delete character before cursor.                 |",
" | Ctrl-D, DEL             |  Delete character under cursor.                  |",
" | Alt-U                   |  Uppercase current or following word.            |",
" | Alt-L                   |  Lowercase current or following word.            |",
" | Alt-C                   |  Capitalize current or following word.           |",
" | Alt-\\                  |  Delete whitespace around cursor.                |",
" | Ctrl-T                  |  Transpose character.                            |",
" +-------------------------+--------------------------------------------------+",
" Cut&Paste Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-K, ESC+End,        |  Cut from cursor to end of line.                 |",
" |   Ctrl-End, Alt-End     |  (Ctrl-End, Alt-End only support Windows/Xterm)  |",
" | Ctrl-U, ESC+Home,       |  Cut from start of line to cursor.               |",
" |   Ctrl-Home, Alt-Home   |  (Ctrl-Home, Alt-Home only support Windows/Xterm)|",
" | Ctrl-X                  |  Cut whole line.                                 |",
" | Alt-Backspace,          |  Cut word to left of cursor.                     |",
" |    Esc+Backspace,       |                                                  |",
" |    Clt-Backspace        |  (Clt-Backspace only supports Windows/Xterm)     |",
" | Alt-D, ESC+Del,         |  Cut word following cursor.                      |",
" |    Alt-Del, Ctrl-Del    |  (Alt-Del,Ctrl-Del only support Windows/Xterm)   |",
" | Ctrl-W                  |  Cut to left till whitespace (not word).         |",
" | Ctrl-Y, Ctrl-V, Insert  |  Paste last cut text.                            |",
" +-------------------------+--------------------------------------------------+",
" Complete Commands",
" +-------------------------+--------------------------------------------------+",
" | TAB, Ctrl-I             |  Autocomplete.                                   |",
" | Alt-=, Alt-?            |  List possible completions.                      |",
" +-------------------------+--------------------------------------------------+",
" History Commands",
" +-------------------------+--------------------------------------------------+",
" | Ctrl-P, Up              |  Fetch previous line in history.                 |",
" | Ctrl-N, Down            |  Fetch next line in history.                     |",
" | Alt-<,  PgUp            |  Move to first line in history.                  |",
" | Alt->,  PgDn            |  Move to end of input history.                   |",
" | Ctrl-R, Ctrl-S          |  Search history.                                 |",
" | F4                      |  Search history with current input.              |",
" | F1                      |  Show search help when in search mode.           |",
" | F2                      |  Show history.                                   |",
" | F3                      |  Clear history (need confirm).                   |",
" +-------------------------+--------------------------------------------------+",
" Control Commands",
" +-------------------------+--------------------------------------------------+",
" | Enter,  Ctrl-J, Ctrl-M  |  EOL and accept line.                            |",
" | Ctrl-C, Ctrl-G          |  EOF and abort line.                             |",
" | Ctrl-D                  |  EOF if line is empty.                           |",
" | Alt-R                   |  Revert line.                                    |",
" | Ctrl-Z                  |  Suspend Job. (Linux Only, fg will resume edit)  |",
" +-------------------------+--------------------------------------------------+",
" Note: If Alt-key doesn't work, an alternate way is to press ESC first then press key, see above ESC+Key.",
" Note: In multiple lines:",
"       Up/Down and Ctrl/Alt-Up, Ctrl/Alt-Down will move between lines.",
"       Up key will fetch history when cursor in first line or end of last line(for quick history move)",
"       Down key will fetch history when cursor in last line.",
"       Ctrl/Alt-Up, Ctrl/Alt-Down will just move between lines.",
NULL};

static char* s_search_help[] = {
"Patterns are separated by ' ', patter match is case insensitive:",
"  (Hint: use Ctrl-Y/Ctrl-V/Insert to paste last paterns)",
"    select:   choose line including 'select'",
"    -select:  choose line excluding 'select'",
"    \"select from\":  choose line including \"select from\"",
"    -\"select from\": choose line excluding \"select from\"",
"Example:",
"    \"select from\" where -\"order by\" -limit:  ",
"         choose line including \"select from\" and 'where'",
"         and excluding \"order by\" or 'limit'",
NULL};

/*****************************************************************************/

// Main API to read a line, return buf if get line, return NULL if EOF.
static char* ax_edit_readline_internal (const char *prompt, char *buf, int size, int has_input)
{
	int not_support = 0, len;

	if ((NULL == buf) || (size <= 1))
		{ return NULL; }
	if (!isatty(STDIN_FILENO)) {  // input is not from a terminal
		not_support = 1;
	} else {
		char *term = getenv("TERM");
		if (NULL != term) {
			if (!strcasecmp(term, "dumb") || !strcasecmp(term, "cons25") ||  !strcasecmp(term, "emacs"))
				{ not_support = 1; }
		}
	}
	if (not_support) {
        if (NULL == fgets(buf, size, stdin))
			{ return NULL; }
        for (len = (int)strlen(buf); (len > 0) && (('\n'==buf[len-1]) || ('\r'==buf[len-1])); --len)
			{ buf[len-1] = '\0'; }
        return buf;
	}

	return ax_edit_readline_edit (buf, size, prompt, has_input, 0);
}
char* ax_edit_readline (const char *prompt, char *buf, int size)
{
	return ax_edit_readline_internal (prompt, buf, size, 0);
}
char* ax_edit_readline2 (const char *prompt, char *buf, int size)
{
	return ax_edit_readline_internal (prompt, buf, size, 1);
}

// Set move/cut word delimiter, defaut is all not digital and alphabetic characters.
void  ax_edit_delimiter_set (const char *delim)
{
	if (NULL != delim) {
		strncpy (s_word_delimiter, delim, sizeof(s_word_delimiter) - 1);
		s_word_delimiter[sizeof(s_word_delimiter) - 1] = '\0';
	}
}

void ax_edit_history_show (void)
{
	ax_edit_history_dump (stdout, 1, NULL, 0, isatty(STDIN_FILENO));
}

void  ax_edit_history_clear (void)
{
	memset (s_history_buf, 0, sizeof (s_history_buf));
	s_history_id = 0;
}

int ax_edit_history_save (const char *filename)
{
	if (NULL == filename) {
		return -1;
	} else {
		FILE *file = fopen(filename, "wt");
		if (file == NULL) {	return -1;	}
		ax_edit_history_dump (file, 0, NULL, 0, 0);
		fclose(file);
	}
	return 0;
}

int ax_edit_history_load (const char* filename)
{
	int		len;
	char	buf[CROSS_HISTORY_BUF_LEN];
	FILE	*file;

	if (NULL == filename)	{	return -1; }
	file = fopen(filename, "rt");
	if (NULL == file)	{ return -1; }
	while (NULL != fgets(buf, CROSS_HISTORY_BUF_LEN, file)) {
        for (len = (int)strlen(buf); (len > 0) && (('\n'==buf[len-1]) || ('\r'==buf[len-1])); --len)
			{ buf[len-1] = '\0'; }
		if (len > 0) {
			buf[CROSS_HISTORY_BUF_LEN-1] = '\0';
			strcpy (s_history_buf[(s_history_id++) % CROSS_HISTORY_MAX_LINE], buf);
		}
	}
	fclose(file);
	return 0;
}

// Register completion callback.
void ax_edit_completion_register (ax_edit_completion_callback pCbFunc)
{
	s_completion_callback = pCbFunc;
}

// Add completion in callback. Word is must, help for word is optional.
void  ax_edit_completion_add_color (ax_edit_completions *pCompletions, const char *word,
		ax_edit_color_e wcolor, const char *help, ax_edit_color_e hcolor)
{
	if ((NULL != pCompletions) && (NULL != word) && (pCompletions->num < CROSS_COMPLET_MAX_LINE)) {
		strncpy (pCompletions->word[pCompletions->num], word, CROSS_COMPLET_WORD_LEN);
		pCompletions->word[pCompletions->num][CROSS_COMPLET_WORD_LEN - 1] = '\0';
		pCompletions->color_word[pCompletions->num] = wcolor;
		pCompletions->help[pCompletions->num][0] = '\0';
		if (NULL != help) {
			strncpy (pCompletions->help[pCompletions->num], help, CROSS_COMPLET_HELP_LEN);
			pCompletions->help[pCompletions->num][CROSS_COMPLET_HELP_LEN - 1] = '\0';
			pCompletions->color_help[pCompletions->num] = hcolor;
		}
		pCompletions->num++;
	}
}
void ax_edit_completion_add (ax_edit_completions *pCompletions, const char *word, const char *help)
{
	ax_edit_completion_add_color (pCompletions, word, AX_EDIT_COLOR_DEFAULT, help, AX_EDIT_COLOR_DEFAULT);
}

// Set syntax hints in callback.
void  ax_edit_hints_set_color (ax_edit_completions *pCompletions, const char *hints, ax_edit_color_e color)
{
	if ((NULL != pCompletions) && (NULL != hints)) {
		strncpy (pCompletions->hints, hints, CROSS_COMPLET_HINT_LEN - 1);
		pCompletions->hints[CROSS_COMPLET_HINT_LEN - 1] = '\0';
		pCompletions->color_hints = color;
	}
}
void ax_edit_hints_set (ax_edit_completions *pCompletions, const char *hints)
{
	ax_edit_hints_set_color (pCompletions, hints, AX_EDIT_COLOR_DEFAULT);
}

/*****************************************************************************/

int ax_edit_paging_set (int enable)
{
	int prev = s_paging_print_line >=0;
	s_paging_print_line = enable ? 0 : -1;
	return prev;
}

int ax_edit_paging_check (int line_len)
{
	char *paging_hints = "*** Press <Space> or <Enter> to continue . . .";
	int	i, ch, rows, cols, len = (int)strlen(paging_hints);

	if ((s_paging_print_line < 0) || !isatty(STDIN_FILENO) || !isatty(STDOUT_FILENO))	{ return 0; }
	ax_edit_screen_get (&rows, &cols);
	s_paging_print_line += (line_len + cols - 1) / cols;
	if (s_paging_print_line >= (rows - 1)) {
		printf ("%s", paging_hints);
		ch = ax_edit_getch();
		if (0 == ch) { ax_edit_getch(); }	// some terminal server may send 0 after Enter
		// clear paging hints
		for (i = 0; i < len; ++i) { printf ("\b"); }
		for (i = 0; i < len; ++i) { printf (" ");  }
		for (i = 0; i < len; ++i) { printf ("\b"); }
		s_paging_print_line = 0;
		if ((' ' != ch) && (KEY_ENTER != ch) && (KEY_ENTER2 != ch)) {
			return 1; 
		}
	}
	return 0;
}

/*****************************************************************************/

void  ax_edit_prompt_color_set (ax_edit_color_e color)
{
	s_prompt_color	= color;
}

void ax_edit_screen_clear ()
{
	int ret = system (s_crossline_win ? "cls" : "clear");
	(void) ret;
}

#ifdef _WIN32	// Windows

int ax_edit_getch (void)
{
	fflush (stdout);
	return _getch();
}
void ax_edit_screen_get (int *pRows, int *pCols)
{
	CONSOLE_SCREEN_BUFFER_INFO inf;
	GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &inf);
	*pCols = inf.srWindow.Right - inf.srWindow.Left + 1;
	*pRows = inf.srWindow.Bottom - inf.srWindow.Top + 1;
	*pCols = *pCols > 1 ? *pCols : 160;
	*pRows = *pRows > 1 ? *pRows : 24;
}
int ax_edit_cursor_get (int *pRow, int *pCol)
{
	CONSOLE_SCREEN_BUFFER_INFO inf;
	GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &inf);
	*pRow = inf.dwCursorPosition.Y - inf.srWindow.Top;
	*pCol = inf.dwCursorPosition.X - inf.srWindow.Left;
	return 0;
}
void ax_edit_cursor_set (int row, int col)
{
	CONSOLE_SCREEN_BUFFER_INFO inf;
	GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &inf);
	inf.dwCursorPosition.Y = (SHORT)row + inf.srWindow.Top;	
	inf.dwCursorPosition.X = (SHORT)col + inf.srWindow.Left;
	SetConsoleCursorPosition (GetStdHandle(STD_OUTPUT_HANDLE), inf.dwCursorPosition);
}
void ax_edit_cursor_move (int row_off, int col_off)
{
	CONSOLE_SCREEN_BUFFER_INFO inf;
	GetConsoleScreenBufferInfo (GetStdHandle(STD_OUTPUT_HANDLE), &inf);
	inf.dwCursorPosition.Y += (SHORT)row_off;
	inf.dwCursorPosition.X += (SHORT)col_off;
	SetConsoleCursorPosition (GetStdHandle(STD_OUTPUT_HANDLE), inf.dwCursorPosition);
}
void ax_edit_cursor_hide (int bHide)
{
	CONSOLE_CURSOR_INFO inf;
	GetConsoleCursorInfo (GetStdHandle(STD_OUTPUT_HANDLE), &inf);
	inf.bVisible = !bHide;
	SetConsoleCursorInfo (GetStdHandle(STD_OUTPUT_HANDLE), &inf);
}

void ax_edit_color_set (ax_edit_color_e color)
{
    CONSOLE_SCREEN_BUFFER_INFO info;
	static WORD dft_wAttributes = 0;
	WORD wAttributes = 0;
	if (!dft_wAttributes) {
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info);
		dft_wAttributes = info.wAttributes;
	}
	if (AX_EDIT_FGCOLOR_DEFAULT == (color&AX_EDIT_FGCOLOR_MASK)) {
		wAttributes |= dft_wAttributes & (FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
	} else {
		wAttributes |= (color&AX_EDIT_FGCOLOR_BRIGHT) ? FOREGROUND_INTENSITY : 0;
		switch (color&AX_EDIT_FGCOLOR_MASK) {
			case AX_EDIT_FGCOLOR_RED:  	wAttributes |= FOREGROUND_RED;	break;
			case AX_EDIT_FGCOLOR_GREEN:  	wAttributes |= FOREGROUND_GREEN;break;
			case AX_EDIT_FGCOLOR_BLUE:  	wAttributes |= FOREGROUND_BLUE;	break;
			case AX_EDIT_FGCOLOR_YELLOW:  wAttributes |= FOREGROUND_RED | FOREGROUND_GREEN;	break;
			case AX_EDIT_FGCOLOR_MAGENTA: wAttributes |= FOREGROUND_RED | FOREGROUND_BLUE;	break;
			case AX_EDIT_FGCOLOR_CYAN:	wAttributes |= FOREGROUND_GREEN | FOREGROUND_BLUE;	break;
			case AX_EDIT_FGCOLOR_WHITE:   wAttributes |= FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;break;
		}
	}
	if (AX_EDIT_BGCOLOR_DEFAULT == (color&AX_EDIT_BGCOLOR_MASK)) {
		wAttributes |= dft_wAttributes & (BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY);
	} else {
		wAttributes |= (color&AX_EDIT_BGCOLOR_BRIGHT) ? BACKGROUND_INTENSITY : 0;
		switch (color&AX_EDIT_BGCOLOR_MASK) {
			case AX_EDIT_BGCOLOR_RED:  	wAttributes |= BACKGROUND_RED;	break;
			case AX_EDIT_BGCOLOR_GREEN:  	wAttributes |= BACKGROUND_GREEN;break;
			case AX_EDIT_BGCOLOR_BLUE:  	wAttributes |= BACKGROUND_BLUE;	break;
			case AX_EDIT_BGCOLOR_YELLOW:  wAttributes |= BACKGROUND_RED | BACKGROUND_GREEN;	break;
			case AX_EDIT_BGCOLOR_MAGENTA: wAttributes |= BACKGROUND_RED | BACKGROUND_BLUE;	break;
			case AX_EDIT_BGCOLOR_CYAN:	wAttributes |= BACKGROUND_GREEN | BACKGROUND_BLUE;	break;
			case AX_EDIT_BGCOLOR_WHITE:   wAttributes |= BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;break;
		}
	}
	if (color & AX_EDIT_UNDERLINE)
		{ wAttributes |= COMMON_LVB_UNDERSCORE; }
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wAttributes);
}

#else // Linux

int ax_edit_getch ()
{
	char ch = 0;
	struct termios old_term, cur_term;
	fflush (stdout);
	if (tcgetattr(STDIN_FILENO, &old_term) < 0)	{ perror("tcsetattr"); }
	cur_term = old_term;
	cur_term.c_lflag &= ~(ICANON | ECHO | ISIG); // echoing off, canonical off, no signal chars
	cur_term.c_cc[VMIN] = 1;
	cur_term.c_cc[VTIME] = 0;
	if (tcsetattr(STDIN_FILENO, TCSANOW, &cur_term) < 0)	{ perror("tcsetattr"); }
	if (read(STDIN_FILENO, &ch, 1) < 0)	{ /* perror("read()"); */ } // signal will interrupt
	if (tcsetattr(STDIN_FILENO, TCSADRAIN, &old_term) < 0)	{ perror("tcsetattr"); }
	return ch;
}
void ax_edit_screen_get (int *pRows, int *pCols)
{
	struct winsize ws = { 0 };
	(void)ioctl (1, TIOCGWINSZ, &ws);
	*pCols = ws.ws_col;
	*pRows = ws.ws_row;
	*pCols = *pCols > 1 ? *pCols : 160;
	*pRows = *pRows > 1 ? *pRows : 24;
}
int ax_edit_cursor_get (int *pRow, int *pCol)
{
	int i;
	char buf[32];
	printf ("\x1b[6n");
	for (i = 0; i < (char)sizeof(buf)-1; ++i) {
		buf[i] = (char)ax_edit_getch ();
		if ('R' == buf[i]) { break; }
	}
	buf[i] = '\0';
	if (2 != sscanf (buf, "\x1b[%d;%dR", pRow, pCol)) { return -1; }
	(*pRow)--; (*pCol)--;
	return 0;
}
void ax_edit_cursor_set (int row, int col)
{
	printf("\x1b[%d;%dH", row+1, col+1);
}
void ax_edit_cursor_move (int row_off, int col_off)
{
	if (col_off > 0)		{ printf ("\x1b[%dC", col_off);  }
	else if (col_off < 0)	{ printf ("\x1b[%dD", -col_off); }
	if (row_off > 0)		{ printf ("\x1b[%dB", row_off);  }
	else if (row_off < 0)	{ printf ("\x1b[%dA", -row_off); }
}
void ax_edit_cursor_hide (int bHide)
{
	printf("\x1b[?25%c", bHide?'l':'h');
}

void ax_edit_color_set (ax_edit_color_e color)
{
	if (!isatty(STDOUT_FILENO))		{ return; }
	printf ("\033[m");
	if (AX_EDIT_FGCOLOR_DEFAULT != (color&AX_EDIT_FGCOLOR_MASK)) 
		{ printf ("\033[%dm", 29 + (color&AX_EDIT_FGCOLOR_MASK) + ((color&AX_EDIT_FGCOLOR_BRIGHT)?60:0)); }
	if (AX_EDIT_BGCOLOR_DEFAULT != (color&AX_EDIT_BGCOLOR_MASK)) 
		{ printf ("\033[%dm", 39 + ((color&AX_EDIT_BGCOLOR_MASK)>>8) + ((color&AX_EDIT_BGCOLOR_BRIGHT)?60:0)); }
	if (color & AX_EDIT_UNDERLINE)
		{ printf ("\033[4m"); }
}

#endif // #ifdef _WIN32

/*****************************************************************************/

static void ax_edit_show_help (int show_search)
{
	int	i;
	char **help = show_search ? s_search_help : s_crossline_help;
 	printf (" \b\n");
	for (i = 0; NULL != help[i]; ++i) {
		printf ("%s\n", help[i]);
		if (ax_edit_paging_check ((int)strlen(help[i])+1))
			{ break; }
	}
}

static void str_to_lower (char *str)
{
	for (; '\0' != *str; ++str)
		{ *str = (char)tolower (*str); }
}

// Match including(no prefix) and excluding(with prefix: '-') patterns.
static int ax_edit_match_patterns (const char *str, char *word[], int num)
{
	int i;
	char buf[CROSS_HISTORY_BUF_LEN];

	strncpy (buf, str, sizeof(buf) - 1);
	buf[sizeof(buf) - 1] = '\0';
	str_to_lower (buf);
	for (i = 0; i < num; ++i) {
		if ('-' == word[i][0]) {
			if (NULL != strstr (buf, &word[i][1]))
				{ return 0; }
		} else if (NULL == strstr (buf, word[i]))
			{ return 0; }
	}
	return 1;
}

// Split pattern string to individual pattern list, handle composite words embraced with " ".
static int ax_edit_split_patterns (char *patterns, char *pat_list[], int max)
{
	int i, num = 0;
	char *pch = patterns;

	if (NULL == patterns) { return 0; }
	while (' ' == *pch)	{ ++pch; }
	while ((num < max) && (NULL != pch)) {
		if (('"' == *pch) || (('-' == *pch) && ('"' == *(pch+1)))) {
			if ('"' != *pch)	{ *(pch+1) = '-'; }
			pat_list[num++] = ++pch;
			if (NULL != (pch = strchr(pch, '"'))) {
				*pch++ = '\0';
				while (' ' == *pch)	{ ++pch; }
			}
		} else {
			pat_list[num++] = pch;
			if (NULL != (pch = strchr (pch, ' '))) {
				*pch = '\0';
				while (' ' == *(++pch))	;
			}
		}
	}
	for (i = 0; i < num; ++i)
		{ str_to_lower (pat_list[i]); }
	return num;
}

// If patterns is not NULL, will filter history.
// If sel_id > 0, return the real id+1 in history buf, else return history number dumped.
static int ax_edit_history_dump (FILE *file, int print_id, char *patterns, int sel_id, int paging)
{
	uint32_t i;
	int		id = 0, num=0;
	char	*pat_list[CROSS_HIS_MATCH_PAT_NUM], *history;

	num = ax_edit_split_patterns (patterns, pat_list, CROSS_HIS_MATCH_PAT_NUM);
	for (i = s_history_id; i < s_history_id + CROSS_HISTORY_MAX_LINE; ++i) {
		history = s_history_buf[i % CROSS_HISTORY_MAX_LINE];
		if ('\0' != history[0]) {
			if ((NULL != patterns) && !ax_edit_match_patterns (history, pat_list, num))
				{ continue; }
			if (sel_id > 0) {
				if (++id == sel_id)
					{ return (i % CROSS_HISTORY_MAX_LINE) + 1; }
				continue;
			}
			if (print_id)	{ fprintf (file, "%4d  %s\n", ++id, history); }
			else			{ fprintf (file, "%s\n", history); }
			if (paging) {
				if (ax_edit_paging_check ((int)strlen(history)+(print_id?7:1)))
					{ break; }
			}
		}
	}
	return id;
}

// Search history, input will be initial search patterns.
static int ax_edit_history_search (char *input)
{
	uint32_t his_id = 0, count;
	char pattern[CROSS_HISTORY_BUF_LEN], buf[8] = "1";

	printf (" \b\n");
	if (NULL != input) {
		strncpy (pattern, input, sizeof(pattern) - 1);
		pattern[sizeof(pattern) - 1] = '\0';
	}
	// Get search patterns
	if (NULL == ax_edit_readline_edit(pattern, sizeof (pattern), "Input Patterns <F1> help: ", (NULL!=input), 1))
		{ return 0; }
	strncpy (s_clip_buf, pattern, sizeof(s_clip_buf) - 1);
	s_clip_buf[sizeof(s_clip_buf) - 1] = '\0';
	count = ax_edit_history_dump (stdout, 1, pattern, 0, 1);
	if (0 == count)	{ return 0; } // Nothing found, just return
	// Get choice
	if (NULL == ax_edit_readline_edit (buf, sizeof (buf), "Input history id: ", (1==count), 1))
		{ return 0; }
	his_id = atoi (buf);
	if (('\0' != buf[0]) && ((his_id > count) || (his_id <= 0))) {
		printf ("Invalid history id: %s\n", buf);
		return 0;
	}
	return ax_edit_history_dump (stdout, 1, pattern, his_id, 0);
}

// Show completions returned by callback.
static int ax_edit_show_completions (ax_edit_completions *pCompletions)
{
	int i, j, ret = 0, word_len = 0, with_help = 0, rows, cols, word_num;

	if (('\0' != pCompletions->hints[0]) || (pCompletions->num > 0)) {
		printf (" \b\n");
		ret = 1;
	}
	// Print syntax hints.
	if ('\0' != pCompletions->hints[0]) {
		printf ("Please input: "); 
		ax_edit_color_set (pCompletions->color_hints);
		printf ("%s", pCompletions->hints); 
		ax_edit_color_set (AX_EDIT_COLOR_DEFAULT);
		printf ("\n");
	}
	if (0 == pCompletions->num)	{ return ret; }
	for (i = 0; i < pCompletions->num; ++i) {
		if ((int)strlen(pCompletions->word[i]) > word_len)
			{ word_len = (int)strlen(pCompletions->word[i]); }
		if ('\0' != pCompletions->help[i][0])	{ with_help = 1; }
	}
	if (with_help) {
		// Print words with help format.
		for (i = 0; i < pCompletions->num; ++i) {
			ax_edit_color_set (pCompletions->color_word[i]);
			printf ("%s", pCompletions->word[i]);
			for (j = 0; j < 4+word_len-(int)strlen(pCompletions->word[i]); ++j)
				{ printf (" "); }
			ax_edit_color_set (pCompletions->color_help[i]);
			printf ("%s", pCompletions->help[i]);
			ax_edit_color_set (AX_EDIT_COLOR_DEFAULT);
			printf ("\n");
			if (ax_edit_paging_check((int)strlen(pCompletions->help[i])+4+word_len+1))
				{ break; }
		}
		return ret;
	}

	// Print words list in multiple columns.
	ax_edit_screen_get (&rows, &cols);
	word_num = (cols - 1 - word_len) / (word_len + 4) + 1;
	for (i = 1; i <= pCompletions->num; ++i) {
		ax_edit_color_set (pCompletions->color_word[i-1]);
		printf ("%s", pCompletions->word[i-1]);
		ax_edit_color_set (AX_EDIT_COLOR_DEFAULT);
		for (j = 0; j < ((i%word_num)?4:0)+word_len-(int)strlen(pCompletions->word[i-1]); ++j)
			{ printf (" "); }
		if (0 == (i % word_num)) {
			printf ("\n");
			if (ax_edit_paging_check (word_len))
				{ return ret; }
		}
	}

	if (pCompletions->num % word_num) { printf ("\n"); }
	return ret;
}

static int ax_edit_updown_move (const char *prompt, int *pCurPos, int *pCurNum, int off, int bForce)
{
	int rows, cols, len = (int)strlen(prompt), cur_pos=*pCurPos;
	ax_edit_screen_get (&rows, &cols);
	if (!bForce && (*pCurPos == *pCurNum))	{ return 0; } // at end of last line
	if (off < 0) {
		if ((*pCurPos+len)/cols == 0) { return 0; } // at first line
		*pCurPos -= cols;
		if (*pCurPos < 0) { *pCurPos = 0; }
		ax_edit_cursor_move (-1, (*pCurPos+len)%cols-(cur_pos+len)%cols);
	} else {
		if ((*pCurPos+len)/cols == (*pCurNum+len)/cols) { return 0; } // at last line
		*pCurPos += cols;
		if (*pCurPos > *pCurNum) { *pCurPos = *pCurNum - 1; } // one char left to avoid history shortcut
		ax_edit_cursor_move (1, (*pCurPos+len)%cols-(cur_pos+len)%cols);
	}
	return 1;
}

// Refreash current print line and move cursor to new_pos.
static void ax_edit_refreash (const char *prompt, char *buf, int *pCurPos, int *pCurNum, int new_pos, int new_num, int bChg)
{
	int i, pos_row, pos_col, len = (int)strlen(prompt);
	static int rows = 0, cols = 0;

	if (bChg || !rows || s_crossline_win) { ax_edit_screen_get (&rows, &cols); }
	if (!bChg) { // just move cursor
		pos_row = (new_pos+len)/cols - (*pCurPos+len)/cols;
		pos_col = (new_pos+len)%cols - (*pCurPos+len)%cols;
		ax_edit_cursor_move (pos_row, pos_col);
	} else {
		buf[new_num] = '\0';
		if (bChg > 1) { // refreash as less as possbile
			printf ("%s", &buf[bChg-1]);
		} else {
			pos_row = (*pCurPos + len) / cols;
			ax_edit_cursor_move (-pos_row, 0);
			ax_edit_color_set (s_prompt_color);
			printf ("\r%s", prompt);
			ax_edit_color_set (AX_EDIT_COLOR_DEFAULT);
			printf ("%s", buf);
		}
		if (!s_crossline_win && new_num>0 && !((new_num+len)%cols)) { printf("\n"); }
		for (i=*pCurNum-new_num; i > 0; --i) { printf (" "); }
		if (!s_crossline_win && *pCurNum>new_num && !((*pCurNum+len)%cols)) { printf("\n"); }
		pos_row = (new_num+len)/cols - (*pCurNum+len)/cols;
		if (pos_row < 0) { ax_edit_cursor_move (pos_row, 0); } 
		printf ("\r");
		pos_row = (new_pos+len)/cols - (new_num+len)/cols;
		ax_edit_cursor_move (pos_row, (new_pos+len)%cols);
	}
	*pCurPos = new_pos;
	*pCurNum = new_num;
}

static void ax_edit_print (const char *prompt, char *buf, int *pCurPos, int *pCurNum, int new_pos, int new_num)
{
	*pCurPos = *pCurNum = 0;
	ax_edit_refreash (prompt, buf, pCurPos, pCurNum, new_pos, new_num, 1);
}

// Copy part text[cut_beg, cut_end] from src to dest
static void ax_edit_text_copy (char *dest, const char *src, int cut_beg, int cut_end)
{
	int len = cut_end - cut_beg;
	len = (len < CROSS_HISTORY_BUF_LEN) ? len : (CROSS_HISTORY_BUF_LEN - 1);
	if (len > 0) {
		memcpy (dest, &src[cut_beg], len);
		dest[len] = '\0';
	}
}

// Copy from history buffer to dest
static void ax_edit_history_copy (const char *prompt, char *buf, int size, int *pos, int *num, int history_id)
{
	strncpy (buf, s_history_buf[history_id % CROSS_HISTORY_MAX_LINE], size - 1);
	buf[size - 1] = '\0';
	ax_edit_refreash (prompt, buf, pos, num, (int)strlen(buf), (int)strlen(buf), 1);
}

/*****************************************************************************/

// Convert ESC+Key to Alt-Key
static int ax_edit_key_esc2alt (int ch)
{
	switch (ch) {
	case KEY_DEL:	ch = KEY_ALT_DEL;	break;
	case KEY_HOME:	ch = KEY_ALT_HOME;	break;
	case KEY_END:	ch = KEY_ALT_END;	break;
	case KEY_UP:	ch = KEY_ALT_UP;	break;
	case KEY_DOWN:	ch = KEY_ALT_DOWN;	break;
	case KEY_LEFT:	ch = KEY_ALT_LEFT;	break;
	case KEY_RIGHT:	ch = KEY_ALT_RIGHT;	break;
	case KEY_BACKSPACE:	ch = KEY_ALT_BACKSPACE;	break;
	}
	return ch;
}

// Map other function keys to main key
static int ax_edit_key_mapping (int ch)
{
	switch (ch) {
#ifndef _WIN32
	case KEY_HOME2:			ch = KEY_HOME;			break;
	case KEY_END2:			ch = KEY_END;			break;
	case KEY_CTRL_UP2:		ch = KEY_CTRL_UP;		break;
	case KEY_CTRL_DOWN2:	ch = KEY_CTRL_DOWN;		break;
	case KEY_CTRL_LEFT2:	ch = KEY_CTRL_LEFT;		break;
	case KEY_CTRL_RIGHT2:	ch = KEY_CTRL_RIGHT;	break;
	case KEY_F1_2:			ch = KEY_F1;			break;
	case KEY_F2_2:			ch = KEY_F2;			break;
	case KEY_F3_2:			ch = KEY_F3;			break;
	case KEY_F4_2:			ch = KEY_F4;			break;
#endif
	case KEY_DEL2:			ch = KEY_BACKSPACE;		break;
	}
	return ch;
}

#ifdef _WIN32	// Windows
// Read a KEY from keyboard, is_esc indicats whether it's a function key.
static int ax_edit_getkey (int *is_esc)
{
	int ch = ax_edit_getch (), esc;
	if ((GetKeyState (VK_CONTROL) & 0x8000) && (KEY_DEL2 == ch)) {
		ch = KEY_CTRL_BACKSPACE;
	} else if ((224 == ch) || (0 == ch)) {
		*is_esc = 1;
		ch = ax_edit_getch ();
		ch = (GetKeyState (VK_MENU) & 0x8000) ? ALT_KEY(ch) : ch + (KEY_ESC<<8);
	} else if (KEY_ESC == ch) { // Handle ESC+Key
		*is_esc = 1;
		ch = ax_edit_getkey (&esc);
		ch = ax_edit_key_esc2alt (ch);
	} else if (GetKeyState (VK_MENU) & 0x8000) {
		*is_esc = 1; ch = ALT_KEY(ch);
	}
	return ch;
}

void ax_edit_winchg_reg (void)	{ }

#else // Linux

// Convert escape sequences to internal special function key
static int ax_edit_get_esckey (int ch)
{
	int ch2;
	if (0 == ch)	{ ch = ax_edit_getch (); }
	if ('[' == ch) {
		ch = ax_edit_getch ();
		if ((ch>='0') && (ch<='6')) {
			ch2 = ax_edit_getch ();
			if ('~' == ch2)	{ ch = ESC_KEY4 (ch, ch2); } // ex. Esc[4~
			else if (';' == ch2) {
				ch2 = ax_edit_getch();
				if (('5' != ch2) && ('3' != ch2))
					{ return 0; }
				ch = ESC_KEY6 (ch, ch2, ax_edit_getch()); // ex. Esc[1;5B
			}
		} else if ('[' == ch) {
			ch = ESC_KEY4 ('[', ax_edit_getch());	// ex. Esc[[A
		} else { ch = ESC_KEY3 (ch); }	// ex. Esc[A
	} else if ('O' == ch) {
		ch = ESC_OKEY (ax_edit_getch());	// ex. EscOP
	} else { ch = ALT_KEY (ch); } // ex. Alt+Backspace
	return ch;
}

// Read a KEY from keyboard, is_esc indicats whether it's a function key.
static int ax_edit_getkey (int *is_esc)
{
	int ch = ax_edit_getch();
	if (KEY_ESC == ch) {
		*is_esc = 1;
		ch = ax_edit_getch ();
		if (KEY_ESC == ch) { // Handle ESC+Key
			ch = ax_edit_get_esckey (0);
			ch = ax_edit_key_mapping (ch);
			ch = ax_edit_key_esc2alt (ch);
		} else { ch = ax_edit_get_esckey (ch); }
	}
	return ch;
}

static void ax_edit_winchg_event (int arg)
{ s_got_resize = 1; }
static void ax_edit_winchg_reg (void)
{
	struct sigaction sa;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	sa.sa_handler = &ax_edit_winchg_event;
	sigaction (SIGWINCH, &sa, NULL);
	s_got_resize = 0;
}

#endif // #ifdef _WIN32

/*****************************************************************************/

/* Internal readline from terminal. has_input indicates buf has inital input.
 * in_his will disable history and complete shortcuts
 */
static char* ax_edit_readline_edit (char *buf, int size, const char *prompt, int has_input, int in_his)
{
	int pos = 0, num = 0, read_end = 0, is_esc;
	int ch, len, new_pos, copy_buf = 0, i, len2;
	uint32_t history_id = s_history_id, search_his;
	char input[CROSS_HISTORY_BUF_LEN];
	ax_edit_completions completions;

	prompt = (NULL != prompt) ? prompt : "";
	if (has_input) {
		num = pos = (int)strlen (buf);
		ax_edit_text_copy (input, buf, pos, num);
	} else
		{ buf[0] = input[0] = '\0'; }
	ax_edit_print (prompt, buf, &pos, &num, pos, num);
	ax_edit_winchg_reg ();

	do {
		is_esc = 0;
		ch = ax_edit_getkey (&is_esc);
		ch = ax_edit_key_mapping (ch);

		if (s_got_resize) { // Handle window resizing for Linux, Windows can handle it automatically
			new_pos = pos;
			ax_edit_refreash (prompt, buf, &pos, &num, 0, num, 0); // goto beginning of line
			printf ("\x1b[J"); // clear to end of screen
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos, num, 1);
			s_got_resize = 0;
		}

		switch (ch) {
/* Misc Commands */
		case KEY_F1:	// Show help
			ax_edit_show_help (in_his);
			ax_edit_print (prompt, buf, &pos, &num, pos, num);
			break;

		case KEY_DEBUG:	// Enter keyboard debug mode
			printf(" \b\nEnter keyboard debug mode, <Ctrl-C> to exit debug\n");
			while (CTRL_KEY('C') != (ch=ax_edit_getch()))
				{ printf ("%3d 0x%02x (%c)\n", ch, ch, isprint(ch) ? ch : ' '); }
			ax_edit_print (prompt, buf, &pos, &num, pos, num);
			break;

/* Move Commands */
		case KEY_LEFT:	// Move back a character.
		case CTRL_KEY('B'):
			if (pos > 0)
				{ ax_edit_refreash (prompt, buf, &pos, &num, pos-1, num, 0); }
			break;

		case KEY_RIGHT:	// Move forward a character.
		case CTRL_KEY('F'):
			if (pos < num)
				{ ax_edit_refreash (prompt, buf, &pos, &num, pos+1, num, 0); }
			break;

		case ALT_KEY('b'):	// Move back a word.
		case ALT_KEY('B'):
		case KEY_CTRL_LEFT:
		case KEY_ALT_LEFT:
			for (new_pos=pos-1; (new_pos > 0) && isdelim(buf[new_pos]); --new_pos)	;
			for (; (new_pos > 0) && !isdelim(buf[new_pos]); --new_pos)	;
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos?new_pos+1:new_pos, num, 0);
			break;

		case ALT_KEY('f'):	 // Move forward a word.
		case ALT_KEY('F'):
		case KEY_CTRL_RIGHT:
		case KEY_ALT_RIGHT:
			for (new_pos=pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)	;
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos, num, 0);
			break;

		case CTRL_KEY('A'):	// Move cursor to start of line.
		case KEY_HOME:
			ax_edit_refreash (prompt, buf, &pos, &num, 0, num, 0);
			break;

		case CTRL_KEY('E'):	// Move cursor to end of line
		case KEY_END:
			ax_edit_refreash (prompt, buf, &pos, &num, num, num, 0);
			break;

		case CTRL_KEY('L'):	// Clear screen and redisplay line
			ax_edit_screen_clear ();
			ax_edit_print (prompt, buf, &pos, &num, pos, num);
			break;

		case KEY_CTRL_UP: // Move to up line
		case KEY_ALT_UP:
			ax_edit_updown_move (prompt, &pos, &num, -1, 1);
			break;

		case KEY_ALT_DOWN: // Move to down line
		case KEY_CTRL_DOWN:
			ax_edit_updown_move (prompt, &pos, &num, 1, 1);
			break;

/* Edit Commands */
		case KEY_BACKSPACE: // Delete char to left of cursor (same with CTRL_KEY('H'))
			if (pos > 0) {
				memmove (&buf[pos-1], &buf[pos], num - pos);
				ax_edit_refreash (prompt, buf, &pos, &num, pos-1, num-1, 1);
			}
			break;

		case KEY_DEL:	// Delete character under cursor
		case CTRL_KEY('D'):
			if (pos < num) {
				memmove (&buf[pos], &buf[pos+1], num - pos - 1);
				ax_edit_refreash (prompt, buf, &pos, &num, pos, num - 1, 1);
			} else if ((0 == num) && (ch == CTRL_KEY('D'))) // On an empty line, EOF
				 { printf (" \b\n"); read_end = -1; }
			break;

		case ALT_KEY('u'):	// Uppercase current or following word.
		case ALT_KEY('U'):
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)
				{ buf[new_pos] = (char)toupper (buf[new_pos]); }
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos, num, 1);
			break;

		case ALT_KEY('l'):	// Lowercase current or following word.
		case ALT_KEY('L'):
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)
				{ buf[new_pos] = (char)tolower (buf[new_pos]); }
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos, num, 1);
			break;

		case ALT_KEY('c'):	// Capitalize current or following word.
		case ALT_KEY('C'):
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			if (new_pos<num)
				{ buf[new_pos] = (char)toupper (buf[new_pos]); }
			for (; new_pos<num && !isdelim(buf[new_pos]); ++new_pos)	;
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos, num, 1);
			break;

		case ALT_KEY('\\'): // Delete whitespace around cursor.
			for (new_pos = pos; (new_pos > 0) && (' ' == buf[new_pos]); --new_pos)	;
			memmove (&buf[new_pos], &buf[pos], num - pos);
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos, num - (pos-new_pos), 1);
			for (new_pos = pos; (new_pos < num) && (' ' == buf[new_pos]); ++new_pos)	;
			memmove (&buf[pos], &buf[new_pos], num - new_pos);
			ax_edit_refreash (prompt, buf, &pos, &num, pos, num - (new_pos-pos), 1);
			break;

		case CTRL_KEY('T'): // Transpose previous character with current character.
			if ((pos > 0) && !isdelim(buf[pos]) && !isdelim(buf[pos-1])) {
				ch = buf[pos];
				buf[pos] = buf[pos-1];
				buf[pos-1] = (char)ch;
				ax_edit_refreash (prompt, buf, &pos, &num, pos<num?pos+1:pos, num, 1);
			} else if ((pos > 1) && !isdelim(buf[pos-1]) && !isdelim(buf[pos-2])) {
				ch = buf[pos-1];
				buf[pos-1] = buf[pos-2];
				buf[pos-2] = (char)ch;
				ax_edit_refreash (prompt, buf, &pos, &num, pos, num, 1);
			}
			break;

/* Cut&Paste Commands */
		case CTRL_KEY('K'): // Cut from cursor to end of line.
		case KEY_CTRL_END:
		case KEY_ALT_END:
			ax_edit_text_copy (s_clip_buf, buf, pos, num);
			ax_edit_refreash (prompt, buf, &pos, &num, pos, pos, 1);
			break;

		case CTRL_KEY('U'): // Cut from start of line to cursor.
		case KEY_CTRL_HOME:
		case KEY_ALT_HOME:
			ax_edit_text_copy (s_clip_buf, buf, 0, pos);
			memmove (&buf[0], &buf[pos], num-pos);
			ax_edit_refreash (prompt, buf, &pos, &num, 0, num - pos, 1);
			break;

		case CTRL_KEY('X'):	// Cut whole line.
			ax_edit_text_copy (s_clip_buf, buf, 0, num);
			// fall through
		case ALT_KEY('r'):	// Revert line
		case ALT_KEY('R'):
			ax_edit_refreash (prompt, buf, &pos, &num, 0, 0, 1);
			break;

		case CTRL_KEY('W'): // Cut whitespace (not word) to left of cursor.
		case KEY_ALT_BACKSPACE: // Cut word to left of cursor.
		case KEY_CTRL_BACKSPACE:
			new_pos = pos;
			if ((new_pos > 1) && isdelim(buf[new_pos-1]))	{ --new_pos; }
			for (; (new_pos > 0) && isdelim(buf[new_pos]); --new_pos)	;
			if (CTRL_KEY('W') == ch) {
				for (; (new_pos > 0) && (' ' != buf[new_pos]); --new_pos)	;
			} else {
				for (; (new_pos > 0) && !isdelim(buf[new_pos]); --new_pos)	;
			}
			if ((new_pos>0) && (new_pos<pos) && isdelim(buf[new_pos]))	{ new_pos++; }
			ax_edit_text_copy (s_clip_buf, buf, new_pos, pos);
			memmove (&buf[new_pos], &buf[pos], num - pos);
			ax_edit_refreash (prompt, buf, &pos, &num, new_pos, num - (pos-new_pos), 1);
			break;

		case ALT_KEY('d'): // Cut word following cursor.
		case ALT_KEY('D'):
		case KEY_ALT_DEL:
		case KEY_CTRL_DEL:
			for (new_pos = pos; (new_pos < num) && isdelim(buf[new_pos]); ++new_pos)	;
			for (; (new_pos < num) && !isdelim(buf[new_pos]); ++new_pos)	;
			ax_edit_text_copy (s_clip_buf, buf, pos, new_pos);
			memmove (&buf[pos], &buf[new_pos], num - new_pos);
			ax_edit_refreash (prompt, buf, &pos, &num, pos, num - (new_pos-pos), 1);
			break;

		case CTRL_KEY('Y'):	// Paste last cut text.
		case CTRL_KEY('V'):
		case KEY_INSERT:
			if ((len=(int)strlen(s_clip_buf)) + num < size) {
				memmove (&buf[pos+len], &buf[pos], num - pos);
				memcpy (&buf[pos], s_clip_buf, len);
				ax_edit_refreash (prompt, buf, &pos, &num, pos+len, num+len, 1);
			}
			break;

/* Complete Commands */
		case KEY_TAB:		// Autocomplete (same with CTRL_KEY('I'))
		case ALT_KEY('='):	// List possible completions.
		case ALT_KEY('?'):
			if (in_his || (NULL == s_completion_callback) || (pos != num))
				{ break; }
			buf[pos] = '\0';
			completions.num = 0;
			completions.hints[0] = '\0';
			s_completion_callback (buf, &completions);
			if (completions.num >= 1) {
				if (KEY_TAB == ch) {
					len2 = len = (int)strlen(completions.word[0]);
					// Find common string for autocompletion
					for (i = 1; (i < completions.num) && (len > 0); ++i) {
						while ((len > 0) && strncasecmp(completions.word[0], completions.word[i], len)) { len--; }
					}
					if (len > 0) {
						if (len2 > num) len2 = num;
						while ((len2 > 0) && strncasecmp(completions.word[0], &buf[num-len2], len2)) { len2--; }
						new_pos = num - len2;
						if (new_pos+i+1 < size) {
							for (i = 0; i < len; ++i) { buf[new_pos+i] = completions.word[0][i]; }
							if (1 == completions.num) { buf[new_pos + (i++)] = ' '; }
							ax_edit_refreash (prompt, buf, &pos, &num, new_pos+i, new_pos+i, 1);
						}
					}
				}
			}
			if (((completions.num != 1) || (KEY_TAB != ch)) && ax_edit_show_completions(&completions))
				{ ax_edit_print (prompt, buf, &pos, &num, pos, num); }
			break;

/* History Commands */
		case KEY_UP:		// Fetch previous line in history.
			if (ax_edit_updown_move (prompt, &pos, &num, -1, 0)) { break; } // check multi line move up
		case CTRL_KEY('P'):
			if (in_his) { break; }
			if (!copy_buf)
				{ ax_edit_text_copy (input, buf, 0, num); copy_buf = 1; }
			if ((history_id > 0) && (history_id+CROSS_HISTORY_MAX_LINE > s_history_id))
				{ ax_edit_history_copy (prompt, buf, size, &pos, &num, --history_id); }
			break;

		case KEY_DOWN:		// Fetch next line in history.
			if (ax_edit_updown_move (prompt, &pos, &num, 1, 0)) { break; } // check multi line move down
		case CTRL_KEY('N'):
			if (in_his) { break; }
			if (!copy_buf)
				{ ax_edit_text_copy (input, buf, 0, num); copy_buf = 1; }
			if (history_id+1 < s_history_id)
				{ ax_edit_history_copy (prompt, buf, size, &pos, &num, ++history_id); }
			else {
				history_id = s_history_id;
				strncpy (buf, input, size - 1);
				buf[size - 1] = '\0';
				ax_edit_refreash (prompt, buf, &pos, &num, (int)strlen(buf), (int)strlen(buf), 1);
			}
			break; //case UP/DOWN

		case ALT_KEY('<'):	// Move to first line in history.
		case KEY_PGUP:
			if (in_his) { break; }
			if (!copy_buf)
				{ ax_edit_text_copy (input, buf, 0, num); copy_buf = 1; }
			if (s_history_id > 0) {
				history_id = s_history_id < CROSS_HISTORY_MAX_LINE ? 0 : s_history_id-CROSS_HISTORY_MAX_LINE;
				ax_edit_history_copy (prompt, buf, size, &pos, &num, history_id);
			}
			break;

		case ALT_KEY('>'):	// Move to end of input history.
		case KEY_PGDN:
			if (in_his) { break; }
			if (!copy_buf)
				{ ax_edit_text_copy (input, buf, 0, num); copy_buf = 1; }
			history_id = s_history_id;
			strncpy (buf, input, size-1);
			buf[size-1] = '\0';
			ax_edit_refreash (prompt, buf, &pos, &num, (int)strlen(buf), (int)strlen(buf), 1);
			break;

		case CTRL_KEY('R'):	// Search history
		case CTRL_KEY('S'):
		case KEY_F4:		// Search history with current input.
			if (in_his) { break; }
			ax_edit_text_copy (input, buf, 0, num);
			search_his = ax_edit_history_search ((KEY_F4 == ch) ? buf : NULL);
			if (search_his > 0)
				{ strncpy (buf, s_history_buf[search_his-1], size-1); }
			else { strncpy (buf, input, size-1); }
			buf[size-1] = '\0';
			ax_edit_print (prompt, buf, &pos, &num, (int)strlen(buf), (int)strlen(buf));
			break;

		case KEY_F2:	// Show history
			if (in_his || (0 == s_history_id)) { break; }
			printf (" \b\n");
			ax_edit_history_show ();
			ax_edit_print (prompt, buf, &pos, &num, pos, num);
			break;

		case KEY_F3:	// Clear history
			if (in_his) { break; }
			printf(" \b\n!!! Confirm to clear history [y]: ");
			if ('y' == ax_edit_getch()) {
				printf(" \b\nHistory are cleared!");
				ax_edit_history_clear ();
				history_id = 0;
			}
			printf (" \b\n");
			ax_edit_print (prompt, buf, &pos, &num, pos, num);
			break;

/* Control Commands */
		case KEY_ENTER:		// Accept line (same with CTRL_KEY('M'))
		case KEY_ENTER2:	// same with CTRL_KEY('J')
			ax_edit_refreash (prompt, buf, &pos, &num, num, num, 0);
			printf (" \b\n");
			read_end = 1;
			break;

		case CTRL_KEY('C'):	// Abort line.
		case CTRL_KEY('G'):
			ax_edit_refreash (prompt, buf, &pos, &num, num, num, 0);
			if (CTRL_KEY('C') == ch)	{ printf (" \b^C\n"); }
			else	{ printf (" \b\n"); }
			num = pos = 0;
			errno = EAGAIN;
			read_end = -1;
			break;;

		case CTRL_KEY('Z'):
#ifndef _WIN32
			raise(SIGSTOP);    // Suspend current process
			ax_edit_print (prompt, buf, &pos, &num, pos, num);
#endif
			break;

		default:
			if (!is_esc && isprint(ch) && (num < size-1)) {
				memmove (&buf[pos+1], &buf[pos], num - pos);
				buf[pos] = (char)ch;
				ax_edit_refreash (prompt, buf, &pos, &num, pos+1, num+1, pos+1);
				copy_buf = 0;
			}
			break;
        } // switch( ch )
	 	fflush(stdout);
	} while ( !read_end );

	if (read_end < 0) { return NULL; }
	if ((num > 0) && (' ' == buf[num-1]))	{ num--; }
	buf[num] = '\0';
	if (!in_his && (num > 0) && strcmp(buf,"history")) { // Save history
		if ((0 == s_history_id) || strncmp (buf, s_history_buf[(s_history_id-1)%CROSS_HISTORY_MAX_LINE], CROSS_HISTORY_BUF_LEN)) {
			strncpy (s_history_buf[s_history_id % CROSS_HISTORY_MAX_LINE], buf, CROSS_HISTORY_BUF_LEN);
			s_history_buf[s_history_id % CROSS_HISTORY_MAX_LINE][CROSS_HISTORY_BUF_LEN - 1] = '\0';
			history_id = ++s_history_id;
			copy_buf = 0;
		}
	}

	return buf;
}

