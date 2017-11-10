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

libfiles += dir.o
libfiles += disk.o
libfiles += entry.o
libfiles += path.o
libfiles += sspec.o

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

tests += dir-test
tests += disk-test
tests += path-test
tests += sspec-test

ifndef NO_VALGRIND
VALGRIND = valgrind --error-exitcode=1 --quiet
endif

.PHONY: all
all: $(libs) $(tests) $(utils)

bmfs: bmfs.c $(libs)

bmfs-cat: bmfs-cat.c $(libs)

bmfs-cp: bmfs-cp.c $(libs)

bmfs-create: bmfs-create.c $(libs)

bmfs-init: bmfs-init.c $(libs)

bmfs-ls: bmfs-ls.c $(libs)

bmfs-rm: bmfs-rm.c $(libs)

dir-test: dir-test.c $(libs)

disk-test: disk-test.c $(libs)

path-test: path-test.c $(libs)

sspec-test: sspec-test.c $(libs)

entry.o: entry.c entry.h limits.h

dir.o: dir.c dir.h entry.h

disk.o: disk.c disk.h dir.h entry.h limits.h

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
	$(RM) $(tests)
	$(RM) $(utils)

.PHONY: test
test:
	$(VALGRIND) ./dir-test
	$(VALGRIND) ./disk-test
	$(VALGRIND) ./path-test
	$(VALGRIND) ./sspec-test

.PHONY: install
install: $(utils) libbmfs.a
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp --update $(utils) $(DESTDIR)$(PREFIX)/bin/
	cp --update libbmfs.a $(DESTDIR)$(PREFIX)/lib/

