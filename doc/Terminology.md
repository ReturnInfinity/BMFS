BMFS Terminology
================

This document lists the various terms used in the BMFS project, what they mean, and how they are different from terms used in other projects.

### Block

In most file systems, a block is the smallest unit of disk allocation space.
For instance, when a file is created, a certain amount of blocks are reserved for the file.
The file is at a certain block within the file system.

While this is partly true for BMFS, blocks are not considered the minimum allocation unit.
Regions of disks are indexed by absolute byte indices. Similarly, the sizes are indicated
by the number of bytes that they occupy. This simplifies many of the calculations and tasks
need to be performed by the file system.

There is still, in a sense, blocks of data in BMFS. The block size is, indeed, the unit of
allocation size. This is only relevant, though, at the time that an allocation is made.
Afterwards, the block size is no longer relevant.

### Header

The header of the file system is a region on disk, usually at the beginning somewhere,
that contains information about how the file system is laid out. It is the first thing
that is read from disk when the file system is imported from the library. In a lot of
file systems, this is also called the superblock.
