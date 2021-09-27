LDFLAGS = -L../lib -lax
 CFLAGS = -Wall -std=c99 -O2 -I../include
   OBJS = $(wildcard *.c)
TARGETS = $(OBJS:.c=)

all: $(TARGETS)
%:%.c
	$(CC) -o $@ $^ -lax
clean:
	$(RM) $(TARGETS)

.PHONY: clean all
