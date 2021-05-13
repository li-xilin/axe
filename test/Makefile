ROOT = ..
CC = gcc
CFLAGS = -I$(ROOT)/include -std=c99 --pedantic -Werror

ifeq ($(DEBUG), 1)
	CFLAGS += -O0 -DAX_DEBUG -g
else
	CFLAGS += -O2
endif

LDFLAGS = -L$(ROOT)/lib\
	  -laxut \
	  -lax \
	  -fsanitize=address

OBJS = test_all.o test_scope.o test_vail.o test_pool.o test_pred.o test_vector.o \
       test_list.o test_avl.o test_hmap.o test_uintk.o test_string.o test_btrie.o \
       test_seq.o test_algo.o test_stack.o test_queue.o test_arr.o

TARGET = test_all


all: $(TARGET)

test: clean $(TARGET)
	./test_all

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)

clean:
	$(RM) $(OBJS) $(TARGET)

.PHONY: all clean test
