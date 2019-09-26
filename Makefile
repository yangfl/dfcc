DEBUG = 1

CPPFLAGS ?= -fdiagnostics-color=always
CFLAGS ?= -Wall -fPIC
LDFLAGS ?=

ifeq ($(DEBUG), 1)
	CFLAGS += -g -DDEBUG
else
	CFLAGS += -Os
	LDFLAGS += -s -flto
endif
CPPFLAGS += -Icommon
CFLAGS += -fms-extensions
LDFLAGS += -rdynamic -fPIE -Wl,--gc-sections

LIBS := glib-2.0 gio-2.0 libsoup-2.4
LIBS_CPPFLAGS := $(shell pkg-config --cflags-only-I $(LIBS))
LIBS_CFLAGS := $(shell pkg-config --cflags-only-other $(LIBS))
LIBS_LDFLAGS := -lxxhash $(shell pkg-config --libs $(LIBS))

CPPFLAGS += -Ivendor/whereami/src

CPPFLAGS += $(LIBS_CPPFLAGS)
CFLAGS += $(LIBS_CFLAGS)
LDFLAGS += $(LIBS_LDFLAGS)

SOURCES := \
	vendor/whereami/src/whereami.c \
	common/broadcast.c common/hexstring.c common/morestring.c \
	config/config.c config/serverurl.c \
	config/source/args.c config/source/default.c config/source/conffile.c \
		config/source/mux.c \
	file/common.c file/cache.c file/entry.c file/etag.c file/hash.c \
	file/localindex.c file/remoteindex.c \
	spawn/hookedsubprocess.c spawn/subprocess.c \
	client/client.c client/ccargs.c client/local.c \
		client/remote.c client/sessionid.c \
	server/server.c server/common.c server/context.c server/debug.c \
		server/handler.c server/job.c server/session.c \
	dfcc.c
OBJS := $(SOURCES:.c=.o)
PREREQUISITES := $(SOURCES:.c=.d)

OUT := out
EXE := $(OUT)/a.out


.PHONY: all
all: $(OUT) $(EXE) $(OUT)/hookfs.so

.PHONY: clean
clean:
	$(MAKE) -C hookfs clean
	rm -rf $(OUT)/
	rm -f $(EXE) $(OBJS) $(PREREQUISITES)

-include $(PREREQUISITES)

%.d: %.c
	$(CC) -M $(CPPFLAGS) $< | sed 's,.*\.o *:,$(<:.c=.o) $@: Makefile ,' > $@

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	ln -sf a.out $(OUT)/b.out

# hookfs
.PHONY: hookfs/hookfs.so
hookfs/hookfs.so:
	$(MAKE) -C hookfs

# install
$(OUT):
	mkdir -p $@

$(OUT)/hookfs.so: hookfs/hookfs.so
	cp $< $@
