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

libfiles += header.o
libfiles += dir.o
libfiles += disk.o
libfiles += encoding.o
libfiles += entry.o
libfiles += errno.o
libfiles += file.o
libfiles += fs.o
libfiles += path.o
libfiles += status.o
libfiles += table.o

stdlibfiles += filedisk.o
stdlibfiles += ramdisk.o
stdlibfiles += time.o
stdlibfiles += size.o

libs += libbmfs.a
ifndef NO_STDLIB
libs += libbmfs-stdlib.a
endif

.PHONY: all
all: $(libs)

entry.o: entry.c entry.h limits.h types.h

errno.o: errno.c errno.h

dir.o: dir.c dir.h entry.h types.h

disk.o: disk.c disk.h dir.h entry.h limits.h types.h

encoding.o: encoding.c encoding.h types.h

file.o: file.c file.h entry.h disk.h types.h

fs.o: fs.c fs.h disk.h entry.h dir.h status.h types.h

header.o: header.c header.h types.h

path.o: path.c path.h types.h

ramdisk.o: ramdisk.c ramdisk.h disk.h dir.h entry.h limits.h

size.o: size.c size.h types.h

status.o: status.c status.h types.h

table.o: table.c table.h disk.h encoding.h types.h

stdlib.o: stdlib.c stdlib.h

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
