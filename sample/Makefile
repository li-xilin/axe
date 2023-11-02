ROOT = ..
include $(ROOT)/config.mak

ifeq ($(TARGET_SYSTEM), win32)
EXT = .exe
else
EXT = .out
endif

# Needed for AIX
CFLAGS += -D _ISOC99_SOURCE=1 -D _ALL_SOURCE=1 -Wno-unused-value
LDFLAGS += -L../lib -lm
SRCS = $(wildcard core/*.c net/*.c kit/*.c ui/*.c)
TARGETS = $(SRCS:.c=$(EXT))

all: $(TARGETS)

core/%$(EXT): LDFLAGS += -laxcore
core/%$(EXT): core/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ifeq ($(TARGET_SYSTEM), win32)
net/%$(EXT): LDFLAGS += -laxnet -laxcore -lws2_32
else
net/%$(EXT): LDFLAGS += -laxnet -laxcore -lpthread
net/%$(EXT): CFLAGS += -D_DEFAULT_SOURCE
endif

net/%$(EXT): net/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ifeq ($(TARGET_SYSTEM), win32)
kit/%$(EXT): LDFLAGS += -laxkit -laxcore
else
kit/%$(EXT): LDFLAGS += -laxkit -laxcore -pthread
kit/%$(EXT): CFLAGS += -D_DEFAULT_SOURCE
endif

kit/%$(EXT): kit/%.c
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)

ifeq ($(TARGET_SYSTEM), win32)
ui/%$(EXT): LDFLAGS += -laxgui -laxcore -lui -luser32 -lgdi32 -lstdc++ -ld2d1 -lcomctl32 -lole32 -ldwrite -luxtheme
ui/%$(EXT): RES_OBJ = ui/resource.o
else
ui/%$(EXT): LDFLAGS += -laxgui -laxcore -lui -lgtk-3 -pthread
ui/%$(EXT): CFLAGS += -D_DEFAULT_SOURCE
endif

ui/%$(EXT): ui/resource.o ui/%.c 
	echo $^
	$(CC) -o $@ $^ $(LDFLAGS) $(CFLAGS)
ui/resource.o: ui/resource.rc
	echo $^
	windres -i $^ -o $@
clean:
	$(RM) $(TARGETS)

.PHONY: clean all
