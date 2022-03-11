LDFLAGS = -L../lib -lax -lm
 CFLAGS = -Wall -std=c99 -O2 -I../include -g -pedantic
   SRCS = $(wildcard *.c container/*.c algorithm/*.c class/*.c)
TARGETS = $(SRCS:.c=.out)

all: $(TARGETS)
%.out:%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
clean:
	$(RM) $(TARGETS)

.PHONY: clean all
