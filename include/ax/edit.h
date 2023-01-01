// Copyright (c) 2019, JC Wang (wang_junchuan@163.com)
#ifndef AX_EDIT_H
#define AX_EDIT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	AX_EDIT_FGCOLOR_DEFAULT   = 0x00,
	AX_EDIT_FGCOLOR_BLACK     = 0x01,
	AX_EDIT_FGCOLOR_RED       = 0x02,
	AX_EDIT_FGCOLOR_GREEN     = 0x03,
	AX_EDIT_FGCOLOR_YELLOW    = 0x04,
	AX_EDIT_FGCOLOR_BLUE      = 0x05,
	AX_EDIT_FGCOLOR_MAGENTA   = 0x06,
	AX_EDIT_FGCOLOR_CYAN      = 0x07,
	AX_EDIT_FGCOLOR_WHITE     = 0x08,
	AX_EDIT_FGCOLOR_BRIGHT    = 0x80,
	AX_EDIT_FGCOLOR_MASK      = 0x7F,

	AX_EDIT_BGCOLOR_DEFAULT   = 0x0000,
	AX_EDIT_BGCOLOR_BLACK     = 0x0100,
	AX_EDIT_BGCOLOR_RED       = 0x0200,
	AX_EDIT_BGCOLOR_GREEN     = 0x0300,
	AX_EDIT_BGCOLOR_YELLOW    = 0x0400,
	AX_EDIT_BGCOLOR_BLUE      = 0x0500,
	AX_EDIT_BGCOLOR_MAGENTA   = 0x0600,
	AX_EDIT_BGCOLOR_CYAN      = 0x0700,
	AX_EDIT_BGCOLOR_WHITE     = 0x0800,
	AX_EDIT_BGCOLOR_BRIGHT    = 0x8000,
	AX_EDIT_BGCOLOR_MASK      = 0x7F00,

	AX_EDIT_UNDERLINE     	  = 0x10000,

	AX_EDIT_COLOR_DEFAULT     = AX_EDIT_FGCOLOR_DEFAULT | AX_EDIT_BGCOLOR_DEFAULT
} ax_edit_color_e;

// Main API to read a line, return buf if get line, return NULL if EOF.
extern char* ax_edit_readline (const char *prompt, char *buf, int size);

// Same with ax_edit_readline except buf holding initial input for editing.
extern char* ax_edit_readline2 (const char *prompt, char *buf, int size);

// Set move/cut word delimiter, default is all not digital and alphabetic characters.
extern void  ax_edit_delimiter_set (const char *delim);

// Read a character from terminal without echo
extern int	 ax_edit_getch (void);


/* 
 * History APIs
 */

// Save history to file
extern int   ax_edit_history_save (const char *filename);

// Load history from file
extern int   ax_edit_history_load (const char *filename);

// Show history in buffer
extern void  ax_edit_history_show (void);

// Clear history
extern void  ax_edit_history_clear (void);


/*
 * Completion APIs
 */

#ifndef AX_EDIT_COMPLETIONS
#define AX_EDIT_COMPLETIONS
typedef struct ax_edit_completions_st ax_edit_completions;
#endif

typedef void (*ax_edit_completion_callback) (const char *buf, ax_edit_completions *pCompletions);

// Register completion callback
extern void  ax_edit_completion_register (ax_edit_completion_callback pCbFunc);

// Add completion in callback. Word is must, help for word is optional.
extern void  ax_edit_completion_add (ax_edit_completions *pCompletions, const char *word, const char *help);

// Add completion with color.
extern void  ax_edit_completion_add_color (ax_edit_completions *pCompletions, const char *word,
		ax_edit_color_e wcolor, const char *help, ax_edit_color_e hcolor);

// Set syntax hints in callback
extern void  ax_edit_hints_set (ax_edit_completions *pCompletions, const char *hints);

// Set syntax hints with color
extern void  ax_edit_hints_set_color (ax_edit_completions *pCompletions, const char *hints, ax_edit_color_e color);


/*
 * Paging APIs
 */

// Enable/Disble paging control
extern int ax_edit_paging_set (int enable);

// Check paging after print a line, return 1 means quit, 0 means continue
// if you know only one line is printed, just give line_len = 1
extern int  ax_edit_paging_check (int line_len);


/* 
 * Cursor APIs
 */

// Get screen rows and columns
extern void ax_edit_screen_get (int *pRows, int *pCols);

// Clear current screen
extern void ax_edit_screen_clear (void);

// Get cursor postion (0 based)
extern int  ax_edit_cursor_get (int *pRow, int *pCol);

// Set cursor postion (0 based)
extern void ax_edit_cursor_set (int row, int col);

// Move cursor with row and column offset, row_off>0 move up row_off lines, <0 move down abs(row_off) lines
// =0 no move for row, similar with col_off
extern void ax_edit_cursor_move (int row_off, int col_off);

// Hide or show cursor
extern void ax_edit_cursor_hide (int bHide);


/* 
 * Color APIs
 */

// Set text color, AX_EDIT_COLOR_DEFAULT will revert to default setting
// `\t` is not supported in Linux terminal, same below. Don't use `\n` in Linux terminal, same below.
extern void ax_edit_color_set (ax_edit_color_e color);

// Set default prompt color
extern void ax_edit_prompt_color_set (ax_edit_color_e color);

#ifdef __cplusplus
}
#endif

#endif

