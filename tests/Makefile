TOP ?= ..

include $(TOP)/make/config.mk
include $(TOP)/make/patterns.mk

VPATH += $(TOP)/include/bmfs
VPATH += $(TOP)/lib

CFLAGS += -Wall -Wextra -Werror -Wfatal-errors -std=gnu99
ifndef BMFS_RELEASE
CFLAGS += -g -O2
else
CFLAGS += -O3
endif

CFLAGS += -I$(TOP)/include

tests += dir-test
tests += disk-test
tests += fs-test
tests += path-test
tests += size-test
tests += sspec-test
tests += table-test

libs += -lbmfs -lbmfs-stdlib

ifndef NO_VALGRIND
VALGRIND = valgrind --error-exitcode=1 --quiet
endif

.PHONY: all
all: $(tests)

dir-test.o: dir-test.c

dir-test: dir-test.o $(libs)

disk-test.o: disk-test.c

disk-test: disk-test.o $(libs)

fs-test.o: fs-test.c

fs-test: fs-test.o $(libs)

path-test.o: path-test.c

path-test: path-test.o $(libs)

size-test.o: size-test.c

sspec-test.o: sspec-test.c

sspec-test: sspec-test.o $(libs)

table-test.o: table-test.c

table-test: table-test.o $(libs)

.PHONY: clean
clean:
	$(RM) $(tests) $(tests:=.o)

.PHONY: test
test:
	$(VALGRIND) ./dir-test
	$(VALGRIND) ./disk-test
	$(VALGRIND) ./fs-test
	$(VALGRIND) ./path-test
	$(VALGRIND) ./sspec-test
	$(VALGRIND) ./table-test

.PHONY: install
install:

$(V).SILENT:
