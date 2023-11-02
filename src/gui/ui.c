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

#include "ui/ui.h"
#include "ax/detect.h"
#include "ax/log.h"
#include <ui.h>
#include <stdbool.h>

#ifdef AX_OS_WIN
#include <windef.h>
#include <libloaderapi.h>
#include <winuser.h>
#include <errhandlingapi.h>

typedef WINAPI UINT (*GetDpiForSystemProc)();

static WINAPI UINT (*_GetDpiForSystem)() = NULL;

static WINAPI UINT DefaultGetDpiForSystem()
{
	return 96;
}

#endif

int ui_init(void)
{
	int retval = -1;
	uiInitOptions opts = { 0 };
	const char *err = uiInit(&opts);
	if (err) {
		ax_perror("uiInit failed: %s", err);
		uiFreeInitError(err);
		goto out;
	}

#ifdef AX_OS_WIN
	HMODULE hUser32 = GetModuleHandleA("user32.dll");

	_GetDpiForSystem = NULL;
	if (hUser32)
		_GetDpiForSystem = (GetDpiForSystemProc)GetProcAddress(hUser32, "GetDpiForSystem");

	if (!_GetDpiForSystem) {
		SetLastError(0);
		_GetDpiForSystem = &DefaultGetDpiForSystem;
	}

	if (!SetProcessDPIAware()) {
		ax_perror("SetProcessDPIAware failed: %d", GetLastError());
		goto out;
	}
#endif
	retval = 0;
out:
	return retval;
}

double ui_scale(void)
{
	double ratio = 1;
#ifdef AX_OS_WIN
	int zoom = _GetDpiForSystem();
	switch (zoom) {
		case 96:
			ratio = 1;
			break;
		case 120:
			ratio = 1.25;
			break;
		case 144:
			ratio = 1.5;
			break;
		case 192:
			ratio = 2;
			break;
	}
#endif
	return ratio;
}

void ui_quit(void)
{
	uiQuit();
}

void ui_exit(void)
{
	uiUninit();
}

void ui_loop(void)
{
	uiMain();
}

void ui_main_steps(void)
{
	uiMainSteps();
}

int ui_main_step(int wait)
{
	return uiMainStep(wait);
}

void ui_queue_main(void (*f)(void *arg), void *arg)
{
	uiQueueMain(f, arg);
}

void ui_timer(int milliseconds, int (*f)(void *arg), void *arg)
{
	uiTimer(milliseconds, f, arg);
}

void ui_on_should_quit(bool (*f)(void *arg), void *arg)
{
	uiOnShouldQuit((int (*)(void *arg))f, arg);
}

void ui_str_free(char *text)
{
	uiFreeText(text);
}

