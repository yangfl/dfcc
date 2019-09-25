CFLAGS ?= -Wall -Os -fPIC -g
LDFLAGS ?=

ifeq ($(DEBUG), 1)
	CFLAGS += -g -DDEBUG
else
	CFLAGS += -Os -flto
	LDFLAGS += -s
endif
CFLAGS += -fms-extensions -Icommon
LDFLAGS += -rdynamic -fPIE -Wl,--gc-sections

LIBS := glib-2.0
CFLAGS += $(shell pkg-config --cflags $(LIBS))
LDFLAGS += -lxxhash $(shell pkg-config --libs $(LIBS))

all: a.out

a.out: common/exception.o common/vtable.o local_hash.o client.o server.o dfcc.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f a.out *.o */*.o
