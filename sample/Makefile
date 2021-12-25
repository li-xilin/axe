LDFLAGS = -L../lib -lax -lm
 CFLAGS = -Wall -std=c99 -O0 -I../include -g
   SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: $(TARGETS)
%:%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
clean:
	$(RM) $(TARGETS)

.PHONY: clean all
