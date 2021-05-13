/*
 * Copyright (c) 2021 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AX_CHECK_H
#define AX_CHECK_H
#include <ax/debug.h>

#define CHECK_PARAM_NULL(_param) ax_assert((_param), "parameter `%s' is NULL", #_param)

#define CHECK_PARAM_VALIDITY(_param, _cond) ax_assert((_cond), "parameter `%s' is invalid", #_param)

#define CHECK_ITERATOR_VALIDITY(_param, _cond) ax_assert((_cond), "iterator `%s' is invalid", #_param)

#define CHECK_TRAIT_VALIDITY(_trait, _fun) ax_assert((_cond), "trait `%s' is not specified", #_trait)

#define CHECK_ITER_COMPARABLE(_it1, _it2) \
{ \
	ax_assert((_it1)->owner == (_it2)->owner, "different owner for two iterators"); \
	ax_assert((_it1)->tr == (_it2)->tr, "different direction for two iterators"); \
}

#define CHECK_ITER_TYPE(_it, _type) ax_assert(ax_one_is(it->owner, _type), "it->owner is not " _type);

#define UNSUPPORTED() ax_assert(ax_false, "trait unsupported");

#endif
