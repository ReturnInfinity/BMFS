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
libfiles += file.o
libfiles += fs.o
libfiles += path.o
libfiles += ramdisk.o
libfiles += table.o

stdlibfiles += stdlib.o
stdlibfiles += time.o
stdlibfiles += size.o

libs += libbmfs.a
libs += libbmfs-stdlib.a

.PHONY: all
all: $(libs)

entry.o: entry.c entry.h limits.h

dir.o: dir.c dir.h entry.h

disk.o: disk.c disk.h dir.h entry.h limits.h

file.o: file.c file.h entry.h disk.h

fs.o: fs.c fs.h disk.h entry.h dir.h

header.o: header.c header.h

path.o: path.c path.h

ramdisk.o: ramdisk.c ramdisk.h disk.h dir.h entry.h limits.h

size.o: size.c size.h

table.o: table.c table.h disk.h

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
