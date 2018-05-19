TOP ?= ..
CONFIG ?= full

include $(TOP)/make/$(CONFIG)-config.mk
include $(TOP)/make/patterns.mk

VPATH += $(TOP)/include/bmfs

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors -std=gnu99
ifndef BMFS_RELEASE
CFLAGS += -g -O2
else
CFLAGS += -O3
endif

CFLAGS += -fPIC
CFLAGS += -I$(TOP)/include

libfiles += crc32.o
libfiles += dir.o
libfiles += disk.o
libfiles += encoding.o
libfiles += entry.o
libfiles += errno.o
libfiles += file.o
libfiles += fs.o
libfiles += header.o
libfiles += host.o
libfiles += memcpy.o
libfiles += path.o
libfiles += status.o
libfiles += table.o

stdlibfiles += filedisk.o
stdlibfiles += ramdisk.o
stdlibfiles += time.o
stdlibfiles += size.o
stdlibfiles += stdhost.o

libs += libbmfs.a
ifndef NO_STDLIB
libs += libbmfs-stdlib.a
endif

.PHONY: all
all: $(libs)

crc32.o: crc32.c crc32.h types.h

dir.o: dir.c dir.h entry.h types.h

disk.o: disk.c disk.h dir.h entry.h limits.h types.h

encoding.o: encoding.c encoding.h types.h

entry.o: entry.c entry.h limits.h memcpy.h crc32.h types.h

errno.o: errno.c errno.h

file.o: file.c file.h entry.h disk.h types.h

fs.o: fs.c fs.h disk.h entry.h dir.h status.h types.h

header.o: header.c header.h types.h crc32.h

host.o: host.c host.h errno.h types.h

memcpy.o: memcpy.c memcpy.h types.h

path.o: path.c path.h types.h

ramdisk.o: ramdisk.c ramdisk.h disk.h dir.h entry.h limits.h

size.o: size.c size.h types.h

stdhost.o: stdhost.c stdhost.h host.h types.h

status.o: status.c status.h types.h

table.o: table.c table.h disk.h encoding.h types.h crc32.h

time.o: time.c time.h

libbmfs.a: $(libfiles)

libbmfs-stdlib.a: $(stdlibfiles)

.PHONY: clean
clean:
	$(RM) libbmfs.a
	$(RM) libbmfs-stdlib.a
	$(RM) $(libfiles)
	$(RM) $(stdlibfiles)

.PHONY: test
test:

.PHONY: install
install: $(utils) $(libs)
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	cp --update $(libs) $(DESTDIR)$(PREFIX)/lib/

$(V).SILENT:
