/*
 * Copyright (c) 2024 Li Xilin <lixilin@gmx.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef AX_LOCK_H
#define AX_LOCK_H

#include <assert.h>
#include <stddef.h>

#ifndef AX_LOCK_DEFINED
#define AX_LOCK_DEFINED
typedef struct ax_lock_st ax_lock;
#endif

#ifndef AX_LOCK_TRAIT_DEFINED
#define AX_LOCK_TRAIT_DEFINED
typedef struct ax_lock_trait_st ax_lock_trait;
#endif

struct ax_lock_st {
    const ax_lock_trait *l_tr;
    void *l_handle;
};

struct ax_lock_trait_st
{
	size_t handle_size;
	int (*t_init)(void *arg);
	int (*t_get)(void *arg);
	int (*t_put)(void *arg);
	void (*t_free)(void *arg);
};


int ax_lock_init(ax_lock *l);

inline static int ax_lock_get(ax_lock *l)
{
	return l->l_tr->t_get(l->l_handle);
}

inline static int ax_lock_put(ax_lock *l)
{
	assert(l);
	assert(l->l_tr);
	assert(l->l_tr->t_put);
	return l->l_tr->t_put(l->l_handle);
}

inline static void ax_lock_free(ax_lock *l)
{
	assert(l);
	assert(l->l_tr);
	assert(l->l_tr->t_free);
	l->l_tr->t_free(l);
}

#endif

