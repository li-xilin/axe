#include "detect.h"
#if defined(AX_OS_UNIX)
#include "unix/mutex.h"
#elif defined(AX_OS_WIN32)
#include "win32/mutex.h"
#else
#error Unsupported platform.
#endif
