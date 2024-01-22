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

#include "ax/errno.h"
#include <ax/uchar.h>
#include <ax/detect.h>
#include <errno.h>
#include <stdlib.h>

#ifdef AX_OS_WIN
#include <errhandlingapi.h>
#include <windows.h>
#endif

#ifndef AX_OS_WIN
static int errno_is_common(int err)
{
	switch (err) {
		case AX_E2BIG:
		case AX_EACCES:
		case AX_EAGAIN:
		case AX_EBADF:
		case AX_EBUSY:
		case AX_ECHILD:
		case AX_EDEADLK:
		case AX_EDOM:
		case AX_EEXIST:
		case AX_EFAULT:
		case AX_EFBIG:
		case AX_EILSEQ:
		case AX_EINTR:
		case AX_EINVAL:
		case AX_EIO:
		case AX_EISDIR:
		case AX_EMFILE:
		case AX_EMLINK:
		case AX_ENAMETOOLONG:
		case AX_ENFILE:
		case AX_ENODEV:
		case AX_ENOENT:
		case AX_ENOEXEC:
		case AX_ENOLCK:
		case AX_ENOMEM:
		case AX_ENOSPC:
		case AX_ENOSYS:
		case AX_ENOTDIR:
		case AX_ENOTEMPTY:
		case AX_ENOTTY:
		case AX_ENXIO:
		case AX_EPERM:
		case AX_EPIPE:
		case AX_ERANGE:
		case AX_EROFS:
		case AX_ESPIPE:
		case AX_ESRCH:
		case AX_EXDEV:
		case AX_EADDRINUSE:
		case AX_EADDRNOTAVAIL:
		case AX_EAFNOSUPPORT:
		case AX_EALREADY:
		case AX_EBADMSG:
		case AX_ECANCELED:
		case AX_ECONNABORTED:
		case AX_ECONNREFUSED:
		case AX_ECONNRESET:
		case AX_EDESTADDRREQ:
		case AX_EHOSTUNREACH:
		case AX_EIDRM:
		case AX_EINPROGRESS:
		case AX_EISCONN:
		case AX_ELOOP:
		case AX_EMSGSIZE:
		case AX_ENETDOWN:
		case AX_ENETRESET:
		case AX_ENETUNREACH:
		case AX_ENOBUFS:
		case AX_ENODATA:
		case AX_ENOLINK:
		case AX_ENOMSG:
		case AX_ENOPROTOOPT:
		case AX_ENOSR:
		case AX_ENOSTR:
		case AX_ENOTCONN:
		case AX_ENOTRECOVERABLE:
		case AX_ENOTSOCK:
		case AX_ENOTSUP:
		case AX_EOVERFLOW:
		case AX_EOWNERDEAD:
		case AX_EPROTO:
		case AX_EPROTONOSUPPORT:
		case AX_EPROTOTYPE:
		case AX_ETIME:
		case AX_ETIMEDOUT:
		case AX_ETXTBSY:
			return 1;
		default:
			return 0;
	}
}
#endif

#ifdef AX_OS_WIN
static int get_errno(int winerror)
{
	// Unwrap FACILITY_WIN32 HRESULT errors.
	if ((winerror & 0xFFFF0000) == 0x80070000) {
		winerror &= 0x0000FFFF;
	}

	// Winsock error codes (10000-11999) are errno values.
	if (winerror >= 10000 && winerror < 12000) {
		switch (winerror) {
			case WSAEINTR:
			case WSAEBADF:
			case WSAEACCES:
			case WSAEFAULT:
			case WSAEINVAL:
			case WSAEMFILE:
				// Winsock definitions of errno values. See WinSock2.h
				return winerror - 10000;
			default:
				return winerror;
		}
	}

	switch (winerror) {
		case ERROR_FILE_NOT_FOUND:
		case ERROR_PATH_NOT_FOUND:
		case ERROR_INVALID_DRIVE:
		case ERROR_NO_MORE_FILES:
		case ERROR_BAD_NETPATH:
		case ERROR_BAD_NET_NAME:
		case ERROR_BAD_PATHNAME:
		case ERROR_FILENAME_EXCED_RANGE:
			return AX_ENOENT;

		case ERROR_BAD_ENVIRONMENT:
			return AX_E2BIG;

		case ERROR_BAD_FORMAT:
		case ERROR_INVALID_STARTING_CODESEG:
		case ERROR_INVALID_STACKSEG:
		case ERROR_INVALID_MODULETYPE:
		case ERROR_INVALID_EXE_SIGNATURE:
		case ERROR_EXE_MARKED_INVALID:
		case ERROR_BAD_EXE_FORMAT:
		case ERROR_ITERATED_DATA_EXCEEDS_64k:
		case ERROR_INVALID_MINALLOCSIZE:
		case ERROR_DYNLINK_FROM_INVALID_RING:
		case ERROR_IOPL_NOT_ENABLED:
		case ERROR_INVALID_SEGDPL:
		case ERROR_AUTODATASEG_EXCEEDS_64k:
		case ERROR_RING2SEG_MUST_BE_MOVABLE:
		case ERROR_RELOC_CHAIN_XEEDS_SEGLIM:
		case ERROR_INFLOOP_IN_RELOC_CHAIN:
			return AX_ENOEXEC;

		case ERROR_INVALID_HANDLE:
		case ERROR_INVALID_TARGET_HANDLE:
		case ERROR_DIRECT_ACCESS_HANDLE:
			return AX_EBADF;

		case ERROR_WAIT_NO_CHILDREN:
		case ERROR_CHILD_NOT_COMPLETE:
			return AX_ECHILD;

		case ERROR_NO_PROC_SLOTS:
		case ERROR_MAX_THRDS_REACHED:
		case ERROR_NESTING_NOT_ALLOWED:
			return AX_EAGAIN;

		case ERROR_ARENA_TRASHED:
		case ERROR_NOT_ENOUGH_MEMORY:
		case ERROR_INVALID_BLOCK:
		case ERROR_NOT_ENOUGH_QUOTA:
			return AX_ENOMEM;

		case ERROR_ACCESS_DENIED:
		case ERROR_CURRENT_DIRECTORY:
		case ERROR_WRITE_PROTECT:
		case ERROR_BAD_UNIT:
		case ERROR_NOT_READY:
		case ERROR_BAD_COMMAND:
		case ERROR_CRC:
		case ERROR_BAD_LENGTH:
		case ERROR_SEEK:
		case ERROR_NOT_DOS_DISK:
		case ERROR_SECTOR_NOT_FOUND:
		case ERROR_OUT_OF_PAPER:
		case ERROR_WRITE_FAULT:
		case ERROR_READ_FAULT:
		case ERROR_GEN_FAILURE:
		case ERROR_SHARING_VIOLATION:
		case ERROR_LOCK_VIOLATION:
		case ERROR_WRONG_DISK:
		case ERROR_SHARING_BUFFER_EXCEEDED:
		case ERROR_NETWORK_ACCESS_DENIED:
		case ERROR_CANNOT_MAKE:
		case ERROR_FAIL_I24:
		case ERROR_DRIVE_LOCKED:
		case ERROR_SEEK_ON_DEVICE:
		case ERROR_NOT_LOCKED:
		case ERROR_LOCK_FAILED:
		case 35:
			return AX_EACCES;

		case ERROR_FILE_EXISTS:
		case ERROR_ALREADY_EXISTS:
			return AX_EEXIST;

		case ERROR_NOT_SAME_DEVICE:
			return AX_EXDEV;

		case ERROR_DIRECTORY:
			return AX_ENOTDIR;

		case ERROR_TOO_MANY_OPEN_FILES:
			return AX_EMFILE;

		case ERROR_DISK_FULL:
			return AX_ENOSPC;

		case ERROR_BROKEN_PIPE:
		case ERROR_NO_DATA:
			return AX_EPIPE;

		case ERROR_DIR_NOT_EMPTY:
			return AX_ENOTEMPTY;

		case ERROR_NO_UNICODE_TRANSLATION:
			return AX_EILSEQ;

		case ERROR_INVALID_FUNCTION:
		case ERROR_INVALID_ACCESS:
		case ERROR_INVALID_DATA:
		case ERROR_INVALID_PARAMETER:
		case ERROR_NEGATIVE_SEEK:
			return AX_EINVAL;
		default:
			return -winerror;

	}
}

#else

static int get_errno(int err)
{
	switch(err) {
		case 0:
			return 0;
		case EDQUOT:
			return AX_EDQUOT;
		default:
			return errno_is_common(err) ? err : -err;
	}
}

#endif

void ax_error_occur(void)
{
	int error = 
#ifdef AX_OS_WIN
		GetLastError();
#else
		errno;
#endif
	if (error)
		errno = get_errno(error);
}

ax_uchar *ax_errmsg(ax_uchar *buf, size_t size)
{
	if (errno >= 0)
		return ax_ustrncpy(buf, ax_strerror(errno), size - 1);
#ifdef AX_OS_WIN
	FormatMessageW(FORMAT_MESSAGE_FROM_SYSTEM, NULL, -errno,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), buf,
			size, NULL);
	return buf;
#else
	return ax_ustrncpy(buf, ax_strerror(-errno), size - 1);

#endif
}

