LDFLAGS = -L../lib -laxe-core -lm
 CFLAGS = -Wall -std=c99 -O2 -I../include -g -pedantic
   SRCS = $(wildcard *.c container/*.c algorithm/*.c class/*.c flow/*.c)
TARGETS = $(SRCS:.c=.out)

all: $(TARGETS)
%.out:%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS) -Wl,-rpath='../../lib'
clean:
	$(RM) $(TARGETS)

.PHONY: clean all
