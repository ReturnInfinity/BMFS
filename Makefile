#!/bin/make -f

# Used by other makefiles to
# find the top directory of the
# project.
TOP=$(CURDIR)

# Indicates which configuration that
# the project is being built with.
CONFIG ?=full

# Export these variables, they are
# needed in the other makefiles.
export TOP
export CONFIG

.PHONY: all clean test install
all clean test install:
	$(MAKE) -C include/bmfs $@
	$(MAKE) -C lib $@
	$(MAKE) -C tests $@
	$(MAKE) -C utils $@

$(V).SILENT:
