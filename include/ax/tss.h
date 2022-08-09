#include "detect/sys.h"
#if defined(AX_OS_UNIX)
#include "unix/tss.h"
#elif defined(AX_OS_WIN32)
#include "win32/tss.h"
#else
#error Unsupported platform.
#endif
