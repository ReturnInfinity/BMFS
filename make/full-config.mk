# DESTDIR:
#   Where to place the all the
#   files. This is mainly used
#   by debian scripts, to place
#   the install in a temporary
#   location.
DESTDIR ?=

# PREFIX:
#   Where to install the project
#   to. By convention, all manually
#   compiled projects are installed
#   into /usr/local.
PREFIX ?= /opt/return-infinity

# NO_UTIL:
#   BMFS comes with a command line
#   tool that is used for creating
#   files and directories. Set this
#   variable to 1 if you don't want
#   this program to be built.
NO_UTIL ?=

# NO_FUSE:
#   BMFS comes with a fuse binding,
#   so that BMFS file systems can be
#   mounted on a host file system in
#   Linux. This variable specifies
#   whether or not the fuse library
#   and headers are available to build
#   the BMFS fuse program.
NO_FUSE ?= 1

# NO_TESTS:
#   By default, test programs are built
#   to ensure that BMFS works as expected
#   on the host machine. Setting this variable
#   to one skips the compilation of these tests.
NO_TESTS ?=

# NO_STDLIB:
#   By default, bindings to the C standard
#   library are compiled, making it easier
#   to use BMFS with the standard C library.
#   Set this variable to one to prevent this
#   from being built.
NO_STDLIB ?=

# VALGRIND:
#   Path to the valgrind executable.
#   If valgrind is within the PATH variable,
#   this may just be 'valgrind'. Valgrind
#   is used to run the test programs, to
#   check for memory errors. The tests may
#   be run without valgrind, so this is
#   disabled by default.
VALGRIND ?=

# BMFS_RELEASE:
#   If this variable defined, compiler optimizations
#   are enabled when compiling the library and programs.
BMFS_RELEASE ?=

# ARFLAGS:
#   These are flags to use when creating and modifying
#   static libraries.
ARFLAGS = rcs

# CROSS_COMPILE:
#   This variable is used to add a cross-compiler prefix
#   to the toolchain programs. An example would be
#   x86_64-none-elf- or aarch64-none-elf-
CROSS_COMPILE ?=

# CC:
#   This is the path or name of the program used to compile
#   C source code.
CC = $(CROSS_COMPILE)gcc

# AR:
#   This is the path or name of the program that is used
#   to create static libraries.
AR = $(CROSS_COMPILE)ar

# LD:
#   This is the name or path of the program that is used
#   to link executables.
LD = $(CROSS_COMPILE)gcc
