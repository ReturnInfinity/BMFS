CFLAGS = -Wall -W -pedantic -std=c99
VPATH = src

.PHONY: all
all: bmfs

bmfs: bmfs.c

.PHONY: clean
clean:
	$(RM) bmfs

