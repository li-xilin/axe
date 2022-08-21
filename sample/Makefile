CFLAGS = -Wall -std=c99 -O2 -I../include -g -pedantic
LDFLAGS = -L../lib -lm
SRCS = $(wildcard core/*.c thread/*.c event/*.c)
TARGETS = $(SRCS:.c=.out)

ifeq ($(OS),Windows_NT)
    SYSTEM = Win32
else
    SYSTEM = $(shell uname -s)
endif

all: $(TARGETS)

core/%.out: LDFLAGS += -laxcore
core/%.out: core/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ifeq ($(SYSTEM), Win32)
event/%.out: LDFLAGS += -laxevent -laxthread -laxcore -lws2_32
else
event/%.out: LDFLAGS += -laxthread -laxevent -laxcore -lpthread
event/%.out: CFLAGS += -D_XOPEN_SOURCE=500
endif

event/%.out: event/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ifneq ($(SYSTEM), Win32)
thread/%.out: CFLAGS += -D_XOPEN_SOURCE=500
endif
thread/%.out: LDFLAGS += -laxthread -laxcore  -lpthread 
thread/%.out: thread/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) $(TARGETS)

.PHONY: clean all
