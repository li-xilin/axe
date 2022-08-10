#include "detect.h"
#if defined(AX_OS_UNIX)
#include "unix/thread.h"
#elif defined(AX_OS_WIN32)
#include "win32/thread.h"
#else
#error Unsupported platform.
#endif
