CFLAGS = -Wall -W -pedantic -std=c99
VPATH = src

.PHONY: all
all: bmfs

bmfs: bmfs.c libbmfs.o

libbmfs.o: libbmfs.c libbmfs.h

.PHONY: clean
clean:
	$(RM) bmfs libbmfs.o

