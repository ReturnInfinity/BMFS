TOP ?= ..

include $(TOP)/make/config.mk
include $(TOP)/make/patterns.mk

VPATH += $(TOP)/include/bmfs

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors -std=gnu99
ifndef BMFS_RELEASE
CFLAGS += -g -O2
else
CFLAGS += -O3
endif

CFLAGS += -fno-stack-protector -fomit-frame-pointer -mno-red-zone -fPIC
CFLAGS += -I$(TOP)/include

libfiles += header.o
libfiles += dir.o
libfiles += disk.o
libfiles += entry.o
libfiles += explorer.o
libfiles += path.o
libfiles += sspec.o
libfiles += ramdisk.o

stdlibfiles += stdlib.o

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

LDLIBS=$(libs)

tests += dir-test
tests += disk-test
tests += path-test
tests += sspec-test
tests += explorer-test

ifndef NO_VALGRIND
VALGRIND = valgrind --error-exitcode=1 --quiet
endif

.PHONY: all
all: $(libs) $(tests) $(utils)

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

explorer-test.o: explorer-test.c

explorer-test: explorer-test.o $(libs)

header.o: header.c header.h

entry.o: entry.c entry.h limits.h

dir.o: dir.c dir.h entry.h

disk.o: disk.c disk.h dir.h entry.h limits.h

ramdisk.o: ramdisk.c ramdisk.h disk.h dir.h entry.h limits.h

path.o: path.c path.h

sspec.o: sspec.c sspec.h

stdlib.o: stdlib.c stdlib.h

libbmfs.a: $(libfiles)

libbmfs-stdlib.a: $(stdlibfiles)

bmfs-fuse: bmfs-fuse.c $(libs)
bmfs-fuse: LDLIBS += $(shell pkg-config --libs fuse)
bmfs-fuse: CFLAGS += $(shell pkg-config --cflags fuse)
bmfs-fuse: CFLAGS += -std=gnu99

.PHONY: clean
clean:
	$(RM) libbmfs.a
	$(RM) libbmfs-stdlib.a
	$(RM) $(libfiles)
	$(RM) $(stdlibfiles)
	$(RM) $(tests) $(tests:=.o)
	$(RM) $(utils) $(utils:=.o)

.PHONY: test
test:
	$(VALGRIND) ./dir-test
	$(VALGRIND) ./disk-test
	$(VALGRIND) ./path-test
	$(VALGRIND) ./sspec-test
	$(VALGRIND) ./explorer-test

.PHONY: install
install: $(utils) $(libs)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp --update $(utils) $(DESTDIR)$(PREFIX)/bin/
	cp --update libbmfs.a $(DESTDIR)$(PREFIX)/lib/

$(V).SILENT:
