DESTDIR ?=
PREFIX ?= /usr/local
CFLAGS = -Wall -W -pedantic -std=c99

ifndef BMFS_RELEASE
CFLAGS += -g -O2
else
CFLAGS += -O3
endif

libfiles += dir.o
libfiles += disk.o
libfiles += entry.o
libfiles += sspec.o

headers += bmfs.h
headers += dir.h
headers += disk.h
headers += entry.h
headers += limits.h
headers += sspec.h

utils += bmfs
utils += bmfs-cat
utils += bmfs-create
utils += bmfs-ls
utils += bmfs-rm

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
all: $(tests) $(utils)

bmfs: bmfs.c libbmfs.a

bmfs-cat: bmfs-cat.c libbmfs.a

bmfs-create: bmfs-create.c libbmfs.a

bmfs-ls: bmfs-ls.c libbmfs.a

bmfs-rm: bmfs-rm.c libbmfs.a

dir-test: dir-test.c libbmfs.a

disk-test: disk-test.c libbmfs.a

sspec-test: sspec-test.c libbmfs.a

entry.o: entry.c entry.h limits.h

dir.o: dir.c dir.h entry.h

disk.o: disk.c disk.h dir.h entry.h limits.h

sspec.o: sspec.c sspec.h

libbmfs.a: $(libfiles)
	$(AR) $(ARFLAGS) $@ $^

bmfs-fuse: bmfs-fuse.c libbmfs.a
bmfs-fuse: LDLIBS += $(shell pkg-config --libs fuse)
bmfs-fuse: CFLAGS += $(shell pkg-config --cflags fuse)
bmfs-fuse: CFLAGS += -std=gnu99

.PHONY: clean
clean:
	$(RM) libbmfs.a
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
	cp bmfs $(DESTDIR)$(PREFIX)/bin/
	cp --update libbmfs.a $(DESTDIR)$(PREFIX)/lib/
	cp --update limits.h entry.h dir.h disk.h bmfs.h $(DESTDIR)$(PREFIX)/include/bmfs/

