# Copyright (c) 2022 Li xilin <lixilin@gmx.com>
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.

ROOT = ../..
include $(ROOT)/config.mak

ifneq ($(TARGET_SYSTEM), win32)
	CFLAGS += -D_DEFAULT_SOURCE
#else
	CFLAGS += -DWIN32_LEAN_AND_MEAN
	CFLAGS += -D_WIN32_WINNT=0x0600
endif

TARGET = $(LIB)/libaxkit.a

OBJS = lib.o edit.o stringbuf.o tcolor.o stat.o sys.o path.o dir.o uchar.o ini.o errno.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) -rcs $@ $(OBJS)
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean
