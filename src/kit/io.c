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

#include "ax/errno.h"
#include "ax/detect.h"

#include <stdio.h>
#include <fcntl.h>

#ifdef AX_OS_WIN
#include <io.h>
#else
#include <unistd.h>
#endif

FILE* ax_fdopen(intptr_t handle, const ax_uchar *mode)
{
#ifdef AX_OS_WIN
	int fd = _open_osfhandle(handle, _O_BINARY);
	if (fd != -1)
		return _wfdopen(fd, mode);
	else
		return NULL;
#else
	FILE *fp = fdopen(handle, mode);
	if (!fp) {
		ax_error_occur();
		return NULL;
	}
	return 0;
#endif
}

int ax_io_setinput(FILE *stream)
{
	fflush(stream);
#ifdef AX_OS_WIN
	fflush(stream);
	return _setmode(fileno(stream), _O_U16TEXT);
#else
	return 0;
#endif
}

int ax_io_setoutput(FILE *stream)
{
	fflush(stream);
#ifdef AX_OS_WIN
	return _setmode(fileno(stream), _O_U8TEXT);
#else
	return 0;
#endif
}

