# Don't build the fuse program.
NO_FUSE ?= 1
# Don't build the command line program.
NO_UTIL ?= 1
# Dont't build the standard C library bindings.
NO_STDLIB ?= 1
# Don't build the test programs.
NO_TESTS ?= 1
# Build the library with the -ffreestanding flag
CFLAGS += -ffreestanding
# If the cross compiler prefix wasn't set, assume
# the caller wants x86_64
CROSS_COMPILE ?= x86_64-none-elf-
# Let the other configuration file fill in the other defaults.
include $(TOP)/make/full-config.mk
