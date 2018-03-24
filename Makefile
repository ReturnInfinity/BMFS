#!/bin/make -f

# used by other makefiles to
# find the top directory of the
# project.
TOP=$(shell pwd)
# exporting it makes it visible
# to other makefiles.
export TOP

.PHONY: all clean test install
all clean test install:
	$(MAKE) -C include/bmfs $@
	$(MAKE) -C lib $@
	$(MAKE) -C tests $@
	$(MAKE) -C utils $@

$(V).SILENT:
