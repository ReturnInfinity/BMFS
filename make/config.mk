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
PREFIX ?= /usr/local

# NO_UNIX_UTILS:
#   BMFS comes with a set of
#   tools that behave very similarly
#   to some of the traditional unix
#   tools (for example, cp). These
#   are not built by default.
NO_UNIX_UTILS ?=

# NO_FUSE:
#   BMFS comes with a fuse binding,
#   so that BMFS file systems can be
#   mounted on a host file system in
#   Linux. This variable specifies
#   whether or not the fuse library
#   and headers are available to build
#   the BMFS fuse program.
NO_FUSE = 1

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

