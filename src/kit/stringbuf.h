/**
 * resizable string buffer
 * (c) 2017-2020 Steve Bennett <steveb@workware.net.au>
 */

#ifndef STRINGBUF_H
#define STRINGBUF_H

/** @file
 * A stringbuf is a resizing, null terminated string buffer.
 *
 * The buffer is reallocated as necessary.
 *
 * In general it is *not* OK to call these functions with a NULL pointer
 * unless stated otherwise.
 *
 * If USE_UTF8 is defined, supports utf8.
 */

/**
 * The stringbuf structure should not be accessed directly.
 * Use the functions below.
 */
typedef struct {
	int remaining;	/**< Allocated, but unused space */
	int last;		/**< Index of the null terminator (and thus the length of the string) */
	int chars;		/**< Count of characters */
	char *data;		/**< Allocated memory containing the string or NULL for empty */
} stringbuf;

stringbuf *sb_alloc(void);

void sb_free(stringbuf *sb);

stringbuf *sb_copy(stringbuf *sb);

static inline int sb_len(stringbuf *sb) {
	return sb->last;
}

static inline int sb_chars(stringbuf *sb) {
#ifdef USE_UTF8
	return sb->chars;
#else
	return sb->last;
#endif
}

void sb_append(stringbuf *sb, const char *str);

void sb_append_len(stringbuf *sb, const char *str, int len);

static inline char *sb_str(const stringbuf *sb)
{
	return sb->data;
}

void sb_insert(stringbuf *sb, int index, const char *str);

/**
 * Delete 'len' bytes in the string at the given index.
 *
 * Any bytes past the end of the buffer are ignored.
 * The buffer remains null terminated.
 *
 * If len is -1, deletes to the end of the buffer.
 */
void sb_delete(stringbuf *sb, int index, int len);

void sb_clear(stringbuf *sb);

char *sb_to_string(stringbuf *sb);

#endif
