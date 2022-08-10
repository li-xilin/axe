#include "detect.h"
#if defined(AX_OS_UNIX)
#include "unix/sem.h"
#elif defined(AX_OS_WIN32)
#include "win32/sem.h"
#else
#error Unsupported platform.
#endif
