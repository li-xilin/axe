LDFLAGS = -L../lib -lax
 CFLAGS = -Wall -std=c99 -O2 -I../include -g
   SRCS = $(wildcard *.c)
TARGETS = $(SRCS:.c=)

all: $(TARGETS)
%:%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
clean:
	$(RM) $(TARGETS)

.PHONY: clean all
