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
#include <ui.h>
#include <stdbool.h>

int ui_init(void)
{
	uiInitOptions opts = { 0 };
	const char *err = uiInit(&opts);
	if (!err)
		return -1;
	uiFreeInitError(err);
	return 0;
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

