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

#include "control.h"
#include "ax/type/one.h"
#include <ax/log.h>
#include <stdlib.h>
#include <stdio.h>

struct control_extends
{
	void (*Destroy)(uiControl *);
	void (*Hide)(uiControl *);
	ax_one *owner;
};

static void destroy_hook(uiControl *c)
{
	struct control_extends *data = (struct control_extends *)(uintptr_t)c->Hide;
	ax_one *owner = data->owner;
	ax_pdebug("free %s", ax_one_name(owner));
	ax_one_free(owner);
}

int control_attach(uiControl *c, ax_one *one)
{
	struct control_extends *data = malloc(sizeof *data);
	if (!data)
		return -1;
	data->Hide = c->Hide;
	data->Destroy = c->Destroy;
	data->owner = one;
	c->Hide = (void (*)(uiControl *))(uintptr_t)data;
	c->Destroy = destroy_hook;
	return 0;
}

ax_one *control_data(uiControl *c)
{
	struct control_extends *data = (struct control_extends *)(uintptr_t)c->Hide;
	return data->owner;
}

void control_detach(uiControl *c)
{
	struct control_extends *data = (struct control_extends *)(uintptr_t)c->Hide;
	if (data) {
		c->Hide = data->Hide;
		c->Destroy = data->Destroy;
		free(data);
	}
}

void control_hide(uiControl *c)
{
	struct control_extends *data = (struct control_extends *)(uintptr_t)c->Hide;
	if (data)
		data->Hide(c);
}

void control_destroy(uiControl *c)
{
	struct control_extends *data = (struct control_extends *)(uintptr_t)c->Hide;
	if (data)
		data->Destroy(c);
}

