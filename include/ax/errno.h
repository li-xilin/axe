/*
 * Copyright (c) 2023 Li xilin <lixilin@gmx.com>
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

#ifndef AX_ERROR_H

#include <ax/detect.h>
#include <ax/uchar.h>
#include <errno.h>
#include <stddef.h>

#define __AX_EBASE 700

#define AX_E2BIG		E2BIG
#define AX_EACCES		EACCES
#define AX_EAGAIN		EAGAIN
#define AX_EBADF		EBADF
#define AX_EBUSY		EBUSY
#define AX_ECHILD		ECHILD
#define AX_EDEADLK		EDEADLK
#define AX_EDEADLOCK		EDEADLOCK
#define AX_EDOM			EDOM
#define AX_EEXIST		EEXIST
#define AX_EFAULT		EFAULT
#define AX_EFBIG		EFBIG
#define AX_EILSEQ		EILSEQ
#define AX_EINTR		EINTR
#define AX_EINVAL		EINVAL
#define AX_EIO			EIO
#define AX_EISDIR		EISDIR
#define AX_EMFILE		EMFILE
#define AX_EMLINK		EMLINK
#define AX_ENAMETOOLONG		ENAMETOOLONG
#define AX_ENFILE		ENFILE
#define AX_ENODEV		ENODEV
#define AX_ENOENT		ENOENT
#define AX_ENOEXEC		ENOEXEC
#define AX_ENOLCK		ENOLCK
#define AX_ENOMEM		ENOMEM
#define AX_ENOSPC		ENOSPC
#define AX_ENOSYS		ENOSYS
#define AX_ENOTDIR		ENOTDIR
#define AX_ENOTEMPTY		ENOTEMPTY
#define AX_ENOTTY		ENOTTY
#define AX_ENXIO		ENXIO
#define AX_EPERM		EPERM
#define AX_EPIPE		EPIPE
#define AX_ERANGE		ERANGE
#define AX_EROFS		EROFS
#define AX_ESPIPE		ESPIPE
#define AX_ESRCH		ESRCH
#define AX_EXDEV		EXDEV
#define AX_EADDRINUSE		EADDRINUSE
#define AX_EADDRNOTAVAIL	EADDRNOTAVAIL
#define AX_EAFNOSUPPORT		EAFNOSUPPORT
#define AX_EALREADY		EALREADY
#define AX_EBADMSG		EBADMSG
#define AX_ECANCELED		ECANCELED
#define AX_ECONNABORTED		ECONNABORTED
#define AX_ECONNREFUSED		ECONNREFUSED
#define AX_ECONNRESET		ECONNRESET
#define AX_EDESTADDRREQ		EDESTADDRREQ
#define AX_EHOSTUNREACH		EHOSTUNREACH
#define AX_EIDRM		EIDRM
#define AX_EINPROGRESS		EINPROGRESS
#define AX_EISCONN		EISCONN
#define AX_ELOOP		ELOOP
#define AX_EMSGSIZE		EMSGSIZE
#define AX_ENETDOWN		ENETDOWN
#define AX_ENETRESET		ENETRESET
#define AX_ENETUNREACH		ENETUNREACH
#define AX_ENOBUFS		ENOBUFS
#define AX_ENODATA		ENODATA
#define AX_ENOLINK		ENOLINK
#define AX_ENOMSG		ENOMSG
#define AX_ENOPROTOOPT		ENOPROTOOPT
#define AX_ENOSR		ENOSR
#define AX_ENOSTR		ENOSTR
#define AX_ENOTCONN		ENOTCONN
#define AX_ENOTRECOVERABLE	ENOTRECOVERABLE
#define AX_ENOTSOCK		ENOTSOCK
#define AX_ENOTSUP		ENOTSUP
#define AX_EOPNOTSUPP		EOPNOTSUPP
#define AX_EOVERFLOW		EOVERFLOW
#define AX_EOWNERDEAD		EOWNERDEAD
#define AX_EPROTO		EPROTO
#define AX_EPROTONOSUPPORT	EPROTONOSUPPORT
#define AX_EPROTOTYPE		EPROTOTYPE
#define AX_ETIME		ETIME
#define AX_ETIMEDOUT		ETIMEDOUT
#define AX_ETXTBSY		ETXTBSY
#define AX_EWOULDBLOCK		EWOULDBLOCK



#define AX_EDQUOT __AX_EBASE + 1

void ax_error_occur(void);

#define AX_ERRBUF_MAX 1024

ax_uchar *ax_strerror(ax_uchar *buf);

#endif
