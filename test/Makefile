ROOT = ..
CC = gcc
CFLAGS = -I$(ROOT)/include -std=c99 --pedantic -Werror

ifneq ($(DEBUG), 0)
	CFLAGS += -O0 -DAX_DEBUG -g
else
	CFLAGS += -O2
endif

LDFLAGS = -L$(ROOT)/lib\
	  -laxut \
	  -lax  #-fsanitize=address

OBJS = t_main.o t_pred.o t_vector.o t_list.o t_avl.o \
       t_hmap.o t_uintk.o t_string.o t_seq.o t_algo.o \
       t_stack.o t_queue.o t_array.o t_btrie.o t_mem.o \
       t_class.o t_stuff.o t_map_impl.o

TARGET = t_all


all: $(TARGET)

test: clean $(TARGET)
	./t_all

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean test
