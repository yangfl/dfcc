DEBUG = 1

CFLAGS ?= -Wall -fPIC
LDFLAGS ?=

ifeq ($(DEBUG), 1)
	CFLAGS += -g -DDEBUG
else
	CFLAGS += -Os
	LDFLAGS += -s -flto
endif
CPPFLAGS += -I../common
CFLAGS += -fms-extensions
LDFLAGS += -shared -ldl

SOURCES := \
	hookfs.c intercept.c
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
