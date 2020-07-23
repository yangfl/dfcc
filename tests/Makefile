CPPFLAGS += -I..

LIBS := gtest_main
LIBS_CPPFLAGS := $(shell pkg-config --cflags-only-I $(LIBS))
LIBS_CANYFLAGS := $(shell pkg-config --cflags-only-other $(LIBS))
LIBS_LDFLAGS := $(shell pkg-config --libs $(LIBS))

CPPFLAGS += $(LIBS_CPPFLAGS)
CANYFLAGS += $(LIBS_CANYFLAGS)
LDFLAGS += $(LIBS_LDFLAGS)

CXXFLAGS += $(CANYFLAGS) -std=c++17

SOURCES := $(wildcard *.cc)
OBJS := $(addprefix ../,$(filter-out dfcc.o,$(DFCC_OBJS))) $(SOURCES:.cc=.o)
PREREQUISITES := $(SOURCES:.cc=.d)

EXE := unittest


.PHONY: all
all: $(EXE)
	./$(EXE)

.PHONY: clean
clean:
	rm -f $(EXE) $(OBJS) $(PREREQUISITES)

-include $(PREREQUISITES)

%.d: %.cc
	$(CXX) -M $(CPPFLAGS) $< | sed 's,.*\.o *:,$(<:.cc=.o) $@: Makefile ,' > $@

$(EXE): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
