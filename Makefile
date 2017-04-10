CFLAGS = -Wall -W -pedantic -std=c99
VPATH = src

.PHONY: all
all: bmfs

ifndef NO_FUSE
all: bmfs-fuse
endif

bmfs: bmfs.c libbmfs.o

bmfs-fuse: bmfs-fuse.c libbmfs.o
bmfs-fuse: LDLIBS += $(shell pkg-config --libs fuse)
bmfs-fuse: CFLAGS += $(shell pkg-config --cflags fuse)
bmfs-fuse: CFLAGS += -std=gnu99

libbmfs.o: libbmfs.c libbmfs.h

.PHONY: clean
clean:
	$(RM) bmfs bmfs-fuse libbmfs.o

