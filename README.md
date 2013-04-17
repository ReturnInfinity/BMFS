# BMFS #


Utility for accessing a disk (or disk image) formatted with BMFS.

## Creating a new disk image ##

Linux/Unix/Mac OS X:

    dd if=/dev/zero of=disk.image bs=1M count=128

Windows:

Download [contig](http://technet.microsoft.com/en-us/sysinternals/bb897428.aspx). Take the size you want in MiB and multiply it by 1048576.

	contig -n disk.image 134217728


## Formatting a disk image ##

	bmfs disk.image format

In Linux/Unix/Mac OS X you can also format a physical drive by passing the correct path.

	sudo bmfs /dev/sdc format

## Display BMFS disk contents ##

	bmfs disk.image list

Sample output:

	C:\baremetal>utils\bmfs BMFS-256-flat.vmdk list
	BMFS-256-flat.vmdk
	Disk Size: 256 MiB
	Name                            |            Size (B)|      Reserved (MiB)
	==========================================================================
	test.app                                           31                    2
	Another file.app                                    1                    2
	helloc.app                                        800                    2

## Create a new file entry and allocate space to it ##

	bmfs disk.image create ThisIsATest.file

You will be prompted for the size to reserve.

## Read from BMFS to a local file##

	bmfs disk.image read ThisIsATest.file

## Write a local file to BMFS ##

	bmfs disk.image write ThisIsATest.file

## Delete a file on BMFS ##

	bmfs disk.image delete ThisIsATest.file

