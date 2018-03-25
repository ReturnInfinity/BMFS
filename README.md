# BMFS

[![Build Status](https://travis-ci.org/ReturnInfinity/BMFS.svg?branch=master)](https://travis-ci.org/ReturnInfinity/BMFS)

Utility for accessing a disk or disk image formatted with BareMetal File System (BMFS).


## Prerequisites

GCC (C compiler) is required for building the BMFS disk utility.

In Ubuntu this can be completed with the following command:

	sudo apt-get install gcc

Optionally, you can also install the Filesystem in Userspace (FUSE) headers. 

	sudo apt-get install libfuse-dev


## Building BMFS

    make

*You can copy the bmfs binary to a location in the system path for ease of use*


## Formatting a New Disk

    bmfs --disk disk.image format 128M

You can also just use the default disk name, `bmfs.img`, and make the command simpler.

    bmfs format 128M

In Linux/Unix/Mac OS X you can also format a physical drive by passing the correct path.

	sudo bmfs /dev/sdc format


## Display BMFS Disk Contents

To list dist contents, use the `ls` command.

	bmfs ls -l

Or specify a path.

	bmfs ls /home -l

Sample output:

	bmfs ls -l
	       0 Mar 24 23:08:13 usr
	       0 Mar 24 23:08:14 home
	       0 Mar 24 23:08:16 lib
	       0 Mar 24 23:08:24 boot

If the command is being run by a script, you can omit the color by using the `--color` option, like this:

	bmfs ls --color never

Aliases for the `ls` command are `dir` and `list`.


## Create a New File

To create a new file without any content on it, you can just use the `touch` command.

	bmfs touch /etc/bashrc

You can also create a new file by copying one from the host file system.

	bmfs cp ./some-script.sh /bin/some-script.sh

An alias for the `cp` command is `copy`.


## Reading File Contents from BMFS

To read a file from BMFS, you can use the `cat` command.

	bmfs cat /home/john/.bashrc

Redirect the output to save it to a file.

	bmfs cat /home/john/.bashrc >.bashrc


## Delete a file on BMFS

To delete a file, use the `rm` command.

	bmfs rm /var/log/unused-file.txt

An alias for the `rm` command is `delete`.
