#include "detect/sys.h"
#if defined(AX_OS_UNIX)
#include "unix/cond.h"
#elif defined(AX_OS_WIN32)
#include "win32/cond.h"
#else
#error Unsupported platform.
#endif
