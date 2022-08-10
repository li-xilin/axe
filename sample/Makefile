LDFLAGS = -L../lib -laxcore -laxthread -lm -pthread
 CFLAGS = -Wall -std=c99 -O2 -I../include -g -pedantic -D_XOPEN_SOURCE=500
   SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=.out)

all: $(TARGETS)
%.out:%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
clean:
	$(RM) $(TARGETS)

.PHONY: clean all
