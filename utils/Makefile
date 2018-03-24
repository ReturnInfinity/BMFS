
TOP ?= ..

include $(TOP)/make/config.mk
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

libs += libbmfs.a
libs += libbmfs-stdlib.a

utils += bmfs
ifndef NO_UNIX_UTILS
utils += bmfs-cat
utils += bmfs-cp
utils += bmfs-create
utils += bmfs-init
utils += bmfs-ls
utils += bmfs-rm
endif

ifndef NO_FUSE
utils += bmfs-fuse
CFLAGS += -D_FILE_OFFSET_BITS=64
endif

.PHONY: all
all: $(utils)

bmfs.o: bmfs.c

bmfs: bmfs.o $(libs)

bmfs-cat.o: bmfs-cat.c

bmfs-cat: bmfs-cat.o $(libs)

bmfs-cp.o: bmfs-cp.c

bmfs-cp: bmfs-cp.o $(libs)

bmfs-create.o: bmfs-create.c

bmfs-create: bmfs-create.o $(libs)

bmfs-init.o: bmfs-init.c

bmfs-init: bmfs-init.o $(libs)

bmfs-ls.o: bmfs-ls.c

bmfs-ls: bmfs-ls.o $(libs)

bmfs-rm.o: bmfs-rm.c

bmfs-rm: bmfs-rm.o $(libs)

dir-test.o: dir-test.c

dir-test: dir-test.o $(libs)

disk-test.o: disk-test.c

disk-test: disk-test.o $(libs)

path-test.o: path-test.c

path-test: path-test.o $(libs)

sspec-test.o: sspec-test.c

sspec-test: sspec-test.o $(libs)

fs-test.o: fs-test.c

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
