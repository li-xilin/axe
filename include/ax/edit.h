/*
 * Copyright (c) 2010, Salvatore Sanfilippo <antirez@gmail.com>
 * Copyright (c) 2010, Pieter Noordhuis <pcnoordhuis@gmail.com>
 * Copyright (c) 2022-2023 Li Xilin <lixilin@gmx.com>
 *
 * Guerrilla line editing library against the idea that a
 * line editing lib needs to be 20,000 lines of C code.
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

#ifndef AX_EDIT_H
#define AX_EDIT_H

#include <stddef.h>

#ifndef NO_COMPLETION
typedef struct ax_edit_completions_st {
  size_t len;
  char **cvec;
} ax_edit_completions;

/*
 * The callback type for tab completion handlers.
 */
typedef void(ax_edit_completion_cb)(const char *prefix, ax_edit_completions *comp, void *arg);

/*
 * Sets the current tab completion handler and returns the previous one, or NULL
 * if no prior one has been set.
 */
ax_edit_completion_cb * ax_edit_set_completion_cb(ax_edit_completion_cb *comp, void *arg);

/*
 * Adds a copy of the given string to the given completion list. The copy is owned
 * by the ax_edit_completions object.
 */
void ax_edit_add_completion(ax_edit_completions *comp, const char *str);

typedef char*(ax_edit_hints_cb)(const char *, int *color, int *bold, void *arg);
void ax_edit_set_hints_cb(ax_edit_hints_cb *callback, void *arg);

typedef void(ax_edit_free_hints_cb)(void *hint, void *arg);
void ax_edit_set_free_hints_cb(ax_edit_free_hints_cb *callback);

#endif

/*
 * Prompts for input using the given string as the input
 * prompt. Returns when the user has tapped ENTER or (on an empty
 * line) EOF (Ctrl-D on Unix, Ctrl-Z on Windows). Returns either
 * a copy of the entered string (for ENTER) or NULL (on EOF).  The
 * caller owns the returned string and must eventually free() it.
 */
char *ax_edit_readline(const char *prompt);

/**
 * Like ax_edit_readline() but starts with an initial buffer.
 */
char *ax_edit_readline2(const char *prompt, const char *initial);

/**
 * Clear the screen.
 */
extern void ax_edit_clear_screen(void);

/*
 * Adds a copy of the given line of the command history.
 */
int ax_edit_history_add(const char *line);

/*
 * Sets the maximum length of the command history, in lines.
 * If the history is currently longer, it will be trimmed,
 * retaining only the most recent entries. If len is 0 or less
 * then this function does nothing.
 */
int ax_edit_history_set_maxlen(int len);

/*
 * Returns the current maximum length of the history, in lines.
 */
int ax_edit_history_maxlen(void);

/*
 * Saves the current contents of the history to the given file.
 * Returns 0 on success.
 */
int ax_edit_history_save(const char *filename);

/*
 * Replaces the current history with the contents
 * of the given file.  Returns 0 on success.
 */
int ax_edit_history_load(const char *filename);

/*
 * Frees all history entries, clearing the history.
 */
void ax_edit_history_free(void);

/*
 * Returns a pointer to the list of history entries, writing its
 * length to *len if len is not NULL. The memory is owned by linenoise
 * and must not be freed.
 */
char **ax_edit_history(int *len);

/*
 * Returns the number of display columns in the current terminal.
 */
int ax_edit_columns(void);

/**
 * Enable or disable multiline mode (disabled by default)
 */
void ax_edit_set_multi_line(int enableml);

#endif

