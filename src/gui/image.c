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

#include "ui/image.h"

#include <ui.h>

#include <stdlib.h>
#include <string.h>

ax_concrete_begin(ui_image)
	uiImage *image;
ax_end;

static void one_free(ax_one *one)
{
	if (!one)
		return;
	ui_image_r self = AX_R_INIT(ax_one, one);
	uiFreeImage(self.ui_image->image);
	free(one);
}

static const char *one_name(const ax_one *one)
{
        return ax_class_name(1, ui_image);
}

const ax_one_trait ui_image_tr =
{
	.free = one_free,
	.name = one_name,
};

ax_concrete_creator(ui_image, const char *filename)
{
	return NULL;
}

ax_concrete_creator(ui_image, double width, double height)
{
        ax_one *one = NULL;
	uiImage *image = NULL;

	image = uiNewImage(width, height);
	if (!image)
		goto fail;

        one = malloc(sizeof(ui_image));
        if (!one)
                goto fail;

        ui_image image_init = {
		.ax_one = {
			.tr = &ui_image_tr,
		},
		.image = image,
        };

        memcpy(one, &image_init, sizeof image_init);
        return one;
fail:
        free(one);
	uiFreeImage(image);
        return NULL;
}

void *ui_image_raw_image(ui_image *t)
{
	return t->image;
}
 
void ui_image_load(ui_image *t, void *pixels, size_t width, size_t height, size_t stride_bytes)
{
	uiImageAppend(t->image, pixels, width, height, stride_bytes);
}

