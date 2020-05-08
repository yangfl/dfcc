DEBUG = 1

CPPFLAGS ?= -fdiagnostics-color=always
CFLAGS ?=
LDFLAGS ?=

CWARN ?= -Wall -Wpointer-arith -Wuninitialized -Wpedantic
CFLAGS += $(CWARN)

ifeq ($(DEBUG), 1)
	CFLAGS += -g -DDEBUG
	LDFLAGS += -rdynamic
else
	CFLAGS += -Os
	LDFLAGS += -s -flto
endif

CPPFLAGS += -I..
CFLAGS += -fms-extensions -fPIC
LDFLAGS += -shared -ldl -Wl,-z,defs -lpthread

SOURCES := \
	hookfs.c serializer.c simpleiostream.c socket.c \
	../common/tinyglib.c
OBJS := $(SOURCES:.c=.o)
PREREQUISITES := $(SOURCES:.c=.d)

EXE := hookfs.so


.PHONY: all
all: $(EXE)

.PHONY: clean
clean:
	rm -f $(EXE) $(OBJS) $(PREREQUISITES)

-include $(PREREQUISITES)

%.d: %.c
	$(CC) -M $(CPPFLAGS) $< | sed 's,.*\.o *:,$(<:.c=.o) $@: Makefile ,' > $@

$(EXE): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
