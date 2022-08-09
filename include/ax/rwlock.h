#include "detect/sys.h"
#if defined(AX_OS_UNIX)
#include "unix/rwlock.h"
#elif defined(AX_OS_WIN32)
#include "win32/rwlock.h"
#else
#error Unsupported platform.
#endif
