# BareMetal File System (BMFS) - Version 1

BMFS is a new file system used by the BareMetal kernel and its related systems. The design is extremely simplified compared to conventional file systems. The system is also geared more toward a small number of very large files (databases, large data files). As all files are contiguous we can also implement memory mapped disk IO. BMFS was inspired by the [RT11 File System](http://en.wikipedia.org/wiki/RT11#File_system).

## Characteristics:

- Very simple layout
- All files are contiguous
- Disk is divided into 2 MiB blocks
- Flat organization; no subdirectories/subfolders

## Disk structure

**Blocks**

For simplicity, BMFS acts as an abstraction layer where a number of contiguous [sectors](http://en.wikipedia.org/wiki/Disk_sector) are accessed instead of individual sectors. With BMFS, each disk block is 2MiB. The disk driver will handle the optimal way to access the disk (based on if the disk uses 512 byte sectors or supports the new [Advanced Format](http://en.wikipedia.org/wiki/Advanced_Format) 4096 byte sectors). 2MiB blocks were chosen to match the 2MiB memory page allocation that is used within BareMetal.

**Free Blocks**

The location of free blocks can be calculated from the directory. As all files are contiguous we can extract the location of free blocks by comparing against the blocks that are currently in use. The calculation for locating free blocks only needs to be completed in the file create function.

**Disk layout**

The first and last disk blocks are reserved for file system usage. All other disk blocks can be used for data.

	Block 0:
	4KiB - Legacy MBR (Master Boot Sector) sector (512B)
		 - Free space (512B)
		 - BMFS marker (512B)
		 - Free space (2560B)
	4KiB - Directory (Max 64 files, 64-bytes for each record)
	The remaining space in Block 0 is free to use.

	Block 1 .. n-1:
	Data

	Block n (last block on disk):
	Copy of Block 0

**Directory**

BMFS supports a single directory with a maximum of 64 individual files. Each file record is 64 bytes. The directory structure is 4096 bytes and starts at sector 8.

**Directory Record structure**:

	Filename (32 bytes) - Null-terminated ASCII string
	Starting Block number (64-bit unsigned int)
	Blocks reserved (64-bit unsigned int)
	File size (64-bit unsigned int)
	Unused (8 bytes)

A file name that starts with 0x00 marks the end of the directory. A file name that starts with 0x01 marks an unused record that should be ignored.

Maximum file size supported is 70,368,744,177,664 bytes (64 TiB) with a maximum of 33,554,432 allocated blocks.

## Files

The following system calls should be available:

- Create (Create and reserve space for a new file)
- Delete (Delete an existing file from the file system)
- Read (Read a file into system memory)
- Write (Write a section of system memory to a file)
- Directory/List (Prepare and display a list of file)
- Query (Query the existence/details of a file)


**Create**

The create function accepts two parameters:

	Name = A null-terminated string no more that 31 characters
	Reserved = The number of blocks to reserve for the file


**Delete**

The delete function accepts one parameter:

	Name = The name of the file to delete


**Read**

The read function accepts two parameters:

	Name = The name of the file to read
	Destination = The memory address to store the file


**Write**

The write function accepts three parameters:

	Name = The name of the file to write
	Source = The memory address of the data
	Size = The amount of bytes to write


**Directory/List**

The dir/ls function accepts no parameters


**Query**

The query function accepts one parameter:

	Name = The name of the file to query

The query function will return the following:

	Size = The current size of the file in bytes
	Reserved = The amount of blocks reserved for the file (0 if it doesn't exist)
