CFLAGS = -Wall -std=c99 -O2 -I../include -g -pedantic
# Needed for AIX
CFLAGS += -D _ISOC99_SOURCE=1 -D _ALL_SOURCE=1 -Wno-unused-value
LDFLAGS = -L../lib -lm
SRCS = $(wildcard core/*.c thread/*.c net/*.c)
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
net/%.out: LDFLAGS += -laxnet -laxthread -laxcore -lws2_32
else
net/%.out: LDFLAGS += -laxthread -laxnet -laxcore -lpthread
net/%.out: CFLAGS += -D_XOPEN_SOURCE=600
endif

net/%.out: net/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ifneq ($(SYSTEM), Win32)
thread/%.out: CFLAGS += -D_XOPEN_SOURCE=600
endif
thread/%.out: LDFLAGS += -laxthread -laxcore -lpthread 
thread/%.out: thread/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) $(TARGETS)

.PHONY: clean all
