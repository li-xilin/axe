/*
 * Copyright (c) 2020 Li hsilin <lihsilyn@gmail.com>
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

#ifndef AXE_LOG_H_
#define AXE_LOG_H_

#define AX_LM_INFO     0
#define AX_LM_WARNING  1
#define AX_LM_ERROR    2

void __ax_log_print (int level, const char*, ...);

#define ax_pinfo(_fmt, ...)    __ax_log_print(AX_LM_INFO, (_fmt), ##__VA_ARGS__)
#define ax_pwarning(_fmt, ...) __ax_log_print(AX_LM_WARNING, (_fmt), ##__VA_ARGS__)
#define ax_perror(_fmt, ...)   __ax_log_print(AX_LM_ERROR, (_fmt), ##__VA_ARGS__)

#define ax_pinfo_if(_cond, _fmt, ...)      ((_cond) ? __ax_log_print(AX_LM_INFO, (_fmt), ##__VA_ARGS__) : (void)0)
#define ax_pwarning_if(_cond, _fmt, ...)   ((_cond) ? __ax_log_print(AX_LM_WARNING, (_fmt), ##__VA_ARGS__) : (void)0)
#define ax_perror_if(_cond, _fmt, ...)     ((_cond) ? __ax_log_print(AX_LM_ERROR, (_fmt), ##__VA_ARGS__) : (void)0)
#define ax_panic_if(_cond, _lm, _fmt, ...) ((_cond) ? __ax_log_print((_lm), (_fmt), ##__VA_ARGS__), abort(): (void)0)

#endif
