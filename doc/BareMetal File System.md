# BareMetal File System (BMFS) - Version 2

BMFS is a new file system used by the BareMetal kernel and its related systems.
The design is extremely simplified compared to conventional file systems.
The system is also geared more toward a small number of very large files (databases, large data files).
As all files are contiguous we can also implement memory mapped disk IO.
BMFS was inspired by the [RT11 File System](http://en.wikipedia.org/wiki/RT11#File_system).


## Characteristics:

- Very simple layout
- All files are contiguous
- Disk is divided into 2 MiB blocks


## Disk Structure

#### Blocks

For simplicity, BMFS acts as an abstraction layer where a number of contiguous [sectors](http://en.wikipedia.org/wiki/Disk_sector) are accessed instead of individual sectors. With BMFS, each disk block is 2MiB. The disk driver will handle the optimal way to access the disk (based on if the disk uses 512 byte sectors or supports the new [Advanced Format](http://en.wikipedia.org/wiki/Advanced_Format) 4096 byte sectors). 2MiB blocks were chosen to match the 2MiB memory page allocation that is used within BareMetal.

#### Disk Layout

The layout begins with a file system header.
The file system header describes where to find the allocation table and root directory, among other things.
Although it's not required, the allocation table usually follows the file system header.
The allocation table tracks all the space allocations for files and directories.
The table currently holds up to 1024 allocations, but is subject to change in the future.
After the allocation table is the root directory.
The root directory is like all other directories, except its name is an empty string.

#### Header Structure

The header is padded 512 bytes. All fields are encoded as little-endian, unsigned integers.

| Offset | Size   | Name            | Description                                           |
|--------|--------|-----------------|-------------------------------------------------------|
| 0x00   | 0x0008 | Signature       | Used to indicate the presence of the header.          |
| 0x08   | 0x0008 | RootOffset      | The offset, in bytes, of the root directory.          |
| 0x10   | 0x0008 | TableOffset     | The offset, in bytes, of the allocation table.        |
| 0x18   | 0x0008 | TableEntryCount | The number of entries used in the allocation table.   |
| 0x20   | 0x0008 | TotalSize       | The total size that the file system may grow to.      |
| 0x28   | 0x0008 | BlockSize       | The number of bytes per block.                        |
| 0x30   | 0x0004 | Flags           | Flags that help describe file system characteristics. |
| 0x34   | 0x0004 | Checksum        | A CRC32 checksum to verify data integrity.            |
| 0x30   | 0x01D0 | Reserved        | Used for padding until it's needed for other fields.  |

#### Allocation Table Entry

An allocation table entry is used to reserve space on disk for files and directories.
Here's the layout of a single entry.

| Offset | Size | Name     | Description                                              |
|--------|------|----------|----------------------------------------------------------|
| 0x00   | 0x08 | Offset   | The offset, in bytes, of the allocation.                 |
| 0x08   | 0x08 | Reserved | The number of bytes reserved for the allocation to grow. |
| 0x10   | 0x04 | Flags    | Flags that relay information about the allocation.       |
| 0x14   | 0x04 | Checksum | A CRC32 checksum to validate the integrity of the entry. |

#### Directory Entry Structure:

A directory can either indicate a file or a directory.
If the entry is a directory, then the data offset points to the entries of the subdirectory.
If the entry is a file, then the data offset points to the file data.

| Offset | Size | Name              | Description                                       |
|--------|------|-------------------|---------------------------------------------------|
| 0x00   | 0xC0 | Entry Name        | The name of the file or directory.                |
| 0xC0   | 0x08 | Data Offset       | The offset, in bytes, of the entry data.          |
| 0xC8   | 0x08 | Data Size         | The size, in bytes, of the entry data.            |
| 0xD0   | 0x08 | Creation Time     | The time that the entry was created.              |
| 0xD8   | 0x08 | Modification Time | The time that the entry data was lasted modified. |
| 0xE0   | 0x08 | Flags             | Indicates entry type and permissions.             |
| 0xE8   | 0x08 | User ID           | The owner's user ID.                              |
| 0xF0   | 0x08 | Group ID          | The group's user ID.                              |
| 0xF8   | 0x08 | Entry Offset      | The offset, in bytes, of this entry structure.    |

