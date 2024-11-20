# BareMetal File System Lite (BMFS-Lite) - Version 1

BMFS-Lite is a new in-memory file system ([RAM Drive](https://en.wikipedia.org/wiki/RAM_drive)) used by BareMetal-OS. Based on BMFS, the design is extremely simplified compared to conventional file systems. The system is also geared more toward a small number of files (utilities, demos).


## Characteristics:

- Very simple layout
- All files are contiguous
- RAM-Disk is divided into 1 KiB blocks
- Flat organization; no subdirectories/subfolders


## RAM-Disk structure

#### Blocks

With BMFS-Lite, each RAM block is 1KiB.

#### Free Blocks

The location of free blocks can be calculated from the directory. As all files are contiguous we can extract the location of free blocks by comparing against the blocks that are currently in use. The calculation for locating free blocks only needs to be completed in the file create function.

#### Disk layout

The first and last disk blocks are reserved for file system usage. All other disk blocks can be used for data.

	Block 0 .. 3:
	4x 1KiB - Directory (Max 64 files, 64-bytes for each record)

	Block 4 .. n:
	Data

#### Directory

BMFS-Lite supports a single directory with a maximum of 64 individual files. Each file record is 64 bytes. The directory structure is 4096 bytes and starts at the first block.

#### Directory Record structure:

	Filename (32 bytes) - Null-terminated ASCII string
	Starting Block number (64-bit unsigned int)
	Blocks reserved (64-bit unsigned int)
	File size (64-bit unsigned int)
	Unused (8 bytes)

A file name that starts with 0x00 marks the end of the directory. A file name that starts with 0x01 marks an unused record that should be ignored.

## Functions

The following system calls should be available:

- Create (Create and reserve space for a new file)
- Delete (Delete an existing file from the file system)
- Read (Read a file into system memory)
- Write (Write a section of system memory to a file)
- Directory/List (Prepare and display a list of file)
- Query (Query the existence/details of a file)


#### Create

The create function accepts two parameters:

	Name = A null-terminated string no more that 31 characters
	Reserved = The number of blocks to reserve for the file


#### Delete

The delete function accepts one parameter:

	Name = The name of the file to delete


#### Read

The read function accepts two parameters:

	Name = The name of the file to read
	Destination = The memory address to store the file


#### Write

The write function accepts three parameters:

	Name = The name of the file to write
	Source = The memory address of the data
	Size = The amount of bytes to write


#### Directory/List

The dir/ls function accepts no parameters


#### Query

The query function accepts one parameter:

	Name = The name of the file to query

The query function will return the following:

	Size = The current size of the file in bytes
	Reserved = The amount of blocks reserved for the file (0 if it doesn't exist)
