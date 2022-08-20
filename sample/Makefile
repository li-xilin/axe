
CFLAGS = -Wall -std=c99 -O2 -I../include -g -pedantic -D_XOPEN_SOURCE=500
LDFLAGS = -L../lib -lm
SRCS = $(wildcard core/*.c thread/*.c event/*.c)
TARGETS = $(SRCS:.c=.out)

all: $(TARGETS)

core/%.out: LDFLAGS += -laxcore
core/%.out: core/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

event/%.out: LDFLAGS += -laxthread -laxevent -laxcore  -lpthread 
event/%.out: event/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

thread/%.out: LDFLAGS += -laxthread -laxcore  -lpthread 
thread/%.out: thread/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

clean:
	$(RM) $(TARGETS)

.PHONY: clean all
