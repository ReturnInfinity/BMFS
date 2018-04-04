TOP ?= ..
CONFIG ?= full

include $(TOP)/make/$(CONFIG)-config.mk
include $(TOP)/make/patterns.mk

VPATH += $(TOP)/include/bmfs
VPATH += $(TOP)/lib

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors -std=gnu99
ifndef BMFS_RELEASE
CFLAGS += -g -O2
else
CFLAGS += -O3
endif

CFLAGS += -I$(TOP)/include

libs += -lbmfs
libs += -lbmfs-stdlib

ifndef NO_UTIL
utils += bmfs
endif

ifndef NO_FUSE
utils += bmfs-fuse
CFLAGS += -D_FILE_OFFSET_BITS=64
endif

.PHONY: all
all: $(utils)

bmfs.o: bmfs.c

bmfs: bmfs.o $(libs)

bmfs-fuse: bmfs-fuse.c $(libs)
bmfs-fuse: LDLIBS += $(shell pkg-config --libs fuse)
bmfs-fuse: CFLAGS += $(shell pkg-config --cflags fuse)
bmfs-fuse: CFLAGS += -std=gnu99

.PHONY: clean
clean:
	$(RM) $(utils) $(utils:=.o)

.PHONY: test
test:

.PHONY: install
install: $(utils) $(libs)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp --update $(utils) $(DESTDIR)$(PREFIX)/bin/

$(V).SILENT:
