DESTDIR ?=
PREFIX ?= /usr/local
CFLAGS = -Wall -W -pedantic -std=c99
NO_UNIX_UTILS = 1

ifndef BMFS_RELEASE
CFLAGS += -g -O2
else
CFLAGS += -O3
endif

libfiles += dir.o
libfiles += disk.o
libfiles += entry.o
libfiles += sspec.o

stdlibfiles += stdlib.o

libs += libbmfs.a
libs += libbmfs-stdlib.a

headers += bmfs.h
headers += dir.h
headers += disk.h
headers += entry.h
headers += limits.h
headers += sspec.h

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
endif

tests += dir-test
tests += disk-test
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

sspec-test: sspec-test.c $(libs)

entry.o: entry.c entry.h limits.h

dir.o: dir.c dir.h entry.h

disk.o: disk.c disk.h dir.h entry.h limits.h

sspec.o: sspec.c sspec.h

stdlib.o: stdlib.c stdlib.h

libbmfs.a: $(libfiles)
	$(AR) $(ARFLAGS) $@ $^

libbmfs-stdlib.a: $(stdlibfiles)
	$(AR) $(ARFLAGS) $@ $^

bmfs-fuse: bmfs-fuse.c $(libs)
bmfs-fuse: LDLIBS += $(shell pkg-config --libs fuse)
bmfs-fuse: CFLAGS += $(shell pkg-config --cflags fuse)
bmfs-fuse: CFLAGS += -std=gnu99

.PHONY: clean
clean:
	$(RM) libbmfs.a
	$(RM) libbmfs-stdlib.a
	$(RM) $(libfiles)
	$(RM) $(tests)
	$(RM) $(utils)

.PHONY: test
test:
	$(VALGRIND) ./dir-test
	$(VALGRIND) ./disk-test
	$(VALGRIND) ./sspec-test

.PHONY: install
install:
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/include/bmfs
	cp $(utils) $(DESTDIR)$(PREFIX)/bin/
	cp --update libbmfs.a $(DESTDIR)$(PREFIX)/lib/
	cp --update $(headers) $(DESTDIR)$(PREFIX)/include/bmfs/

