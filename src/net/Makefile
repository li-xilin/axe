# Copyright (c) 2022 Li hsilin <lihsilyn@gmail.com>
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

OBJS = event.o event_ht.o reactor.o socket.o timer.o

ifneq ($(TARGET_SYSTEM), win32)
CFLAGS += -D_XOPEN_SOURCE=600
endif

ifeq ($(HAVE_EPOLL),yes)
	OBJS += polling_epoll.o
	ifeq ($(TARGET_SYSTEM),win32)
		OBJS += wepoll.o
	endif
else ifeq ($(HAVE_KQUEUE),yes)
	OBJS += polling_kqueue.o
else ifeq ($(HAVE_POLL),yes)
	OBJS += polling_poll.o
else ifeq ($(HAVE_SELECT),yes)
	OBJS += polling_select.o
else
$(error Multiplexing calls are not found in system)
endif

TARGET = $(LIB)/libaxnet.a

all: $(TARGET)

$(TARGET): $(OBJS)
	$(AR) -rcs $@ $(OBJS)
clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean exit
