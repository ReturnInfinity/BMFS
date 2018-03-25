# BareMetal File System (BMFS) - Version 1

BMFS is a new file system used by the BareMetal kernel and its related systems.
The design is extremely simplified compared to conventional file systems.
The system is also geared more toward a small number of very large files (databases, large data files).
As all files are contiguous we can also implement memory mapped disk IO.
BMFS was inspired by the [RT11 File System](http://en.wikipedia.org/wiki/RT11#File_system).


## Characteristics:

- Very simple layout
- All files are contiguous
- Disk is divided into 2 MiB blocks


## Disk structure

#### Blocks

For simplicity, BMFS acts as an abstraction layer where a number of contiguous [sectors](http://en.wikipedia.org/wiki/Disk_sector) are accessed instead of individual sectors. With BMFS, each disk block is 2MiB. The disk driver will handle the optimal way to access the disk (based on if the disk uses 512 byte sectors or supports the new [Advanced Format](http://en.wikipedia.org/wiki/Advanced_Format) 4096 byte sectors). 2MiB blocks were chosen to match the 2MiB memory page allocation that is used within BareMetal.

#### Disk layout

The first and last disk blocks are reserved for file system usage. All other disk blocks can be used for data.

#### Directory Record structure:
