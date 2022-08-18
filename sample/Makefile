CFLAGS = -Wall -std=c99 -O2 -I../include -g -pedantic -D_XOPEN_SOURCE=500

CORE_LDFLAGS = -L../lib -laxcore -laxthread -lm -pthread
CORE_SRCS = $(wildcard core/*.c)
CORE_TARGETS = $(CORE_SRCS:.c=.out)


EVENT_LDFLAGS = -L../lib -laxthread -laxevent -laxcore  -lpthread 
EVENT_SRCS = $(wildcard event/*.c)
EVENT_TARGETS = $(EVENT_SRCS:.c=.out)

all: $(CORE_TARGETS) $(EVENT_TARGETS)

%.out:%.c
	$(CC) -o $@ $^ $(CORE_LDFLAGS) $(CFLAGS)

event/%.out:event/%.c
	$(CC) -o $@ $^ $(EVENT_LDFLAGS) $(CFLAGS)

clean:
	$(RM) $(CORE_TARGETS) $(EVENT_TARGETS)

.PHONY: clean all

