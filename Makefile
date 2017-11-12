#!/bin/make -f

# used by other makefiles to
# find the top directory of the
# project.
TOP=$(shell pwd)
# exporting it makes it visible
# to other makefiles.
export TOP

.PHONY: all
all:
	$(MAKE) -C src all
	$(MAKE) -C include/bmfs all

.PHONY: clean
clean:
	$(MAKE) -C src clean
	$(MAKE) -C include/bmfs clean

.PHONY: test
test:
	$(MAKE) -C src test
	$(MAKE) -C include/bmfs test

.PHONY: install
install:
	$(MAKE) -C src install
	$(MAKE) -C include/bmfs install

$(V).SILENT:
