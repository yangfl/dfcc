DEBUG = 1

CPPFLAGS ?= -fdiagnostics-color=always
CANYFLAGS ?= -fPIC
LDFLAGS ?= -fPIE -Wl,--gc-sections

CWARN ?= -Wall -Wpointer-arith -Wuninitialized -Wpedantic
CANYFLAGS += $(CWARN)

ifeq ($(DEBUG), 1)
	CANYFLAGS += -g -DDEBUG
	LDFLAGS += -rdynamic
else
	CANYFLAGS += -Os
	LDFLAGS += -s -flto
endif

CPPFLAGS += -I. -D_DEFAULT_SOURCE
CANYFLAGS += -fms-extensions
LDFLAGS +=

LIBS := glib-2.0 gio-2.0 gio-unix-2.0 libsoup-2.4 libxxhash whereami
LIBS_CPPFLAGS := $(shell pkg-config --cflags-only-I $(LIBS))
LIBS_CANYFLAGS := $(shell pkg-config --cflags-only-other $(LIBS))
LIBS_LDFLAGS := $(shell pkg-config --libs $(LIBS)) -lpthread

CPPFLAGS += $(LIBS_CPPFLAGS)
CANYFLAGS += $(LIBS_CANYFLAGS)
LDFLAGS += $(LIBS_LDFLAGS)

export CPPFLAGS
export CANYFLAGS
export LDFLAGS

CFLAGS += $(CANYFLAGS) -std=c18

SOURCES := \
	common/broadcast.c common/hexstring.c common/morestring.c \
	common/atomiccount.c common/typeinfo.c \
		common/wrapper/errno.c common/wrapper/file.c common/wrapper/gvariant.c \
		common/wrapper/mappedfile.c common/wrapper/soup.c common/wrapper/threads.c \
	\
	config/config.c config/serverurl.c \
		config/source/args.c config/source/default.c config/source/conffile.c \
		config/source/mux.c \
	\
	file/cache.c file/cacheentry.c file/entry.c file/stat.c file/hash.c \
	file/localindex.c file/remoteindex.c \
	\
	spawn/hookfsserver.c spawn/hookedprocess.c spawn/hookedprocessgroup.c \
		spawn/process.c \
	\
	cc/ccargs.c cc/resultinfo.c \
	\
	client/client.c client/local.c \
	client/remote.c client/prepost.c client/sessionid.c \
	\
	server/server.c server/context.c server/debug.c server/session.c \
		server/handler/middleware.c server/handler/download.c \
		server/handler/homepage.c server/handler/info.c server/handler/rpc.c \
		server/handler/upload.c \
			server/handler/rpc/associate.c server/handler/rpc/submit.c \
			server/handler/rpc/query.c \
	\
	dfcc.c
OBJS := $(SOURCES:.c=.o)
PREREQUISITES := $(SOURCES:.c=.d)

DFCC_OBJS := $(OBJS)
export DFCC_OBJS

OUT := out
EXE := $(OUT)/dfcc


.PHONY: all
all: $(OUT) $(EXE) $(OUT)/hookfs.so

.PHONY: test
test: all
	+$(MAKE) -C tests

.PHONY: cloc
cloc:
	cloc --exclude-dir=vendor,html,tmp,out --exclude-ext=d .

.PHONY: doc
doc:
	doxygen

.PHONY: clean
clean:
	+$(MAKE) -C tests clean
	+$(MAKE) -C hookfs clean
	rm -rf $(OUT)/
	rm -f $(EXE) $(OBJS) $(PREREQUISITES)

-include $(PREREQUISITES)

%.d: %.c
	$(CC) -M $(CPPFLAGS) $< | sed 's,.*\.o *:,$(<:.c=.o) $@: Makefile ,' > $@

$(EXE): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
	ln -sf dfcc $(OUT)/cc

# hookfs
.PHONY: hookfs/hookfs.so
hookfs/hookfs.so:
	+$(MAKE) -C hookfs

# install
$(OUT):
	mkdir -p $@

$(OUT)/hookfs.so: hookfs/hookfs.so
	cp $< $@
