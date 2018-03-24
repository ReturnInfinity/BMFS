/* BareMetal File System Utility */
/* Written by Ian Seyler of Return Infinity */
/* v1.3.0 (2017 10 11) */

#include <bmfs/dir.h>
#include <errno.h>

void bmfs_dir_init(struct BMFSDir *dir)
{
	dir->Disk = NULL;
	bmfs_entry_init(&dir->Entry);
	bmfs_entry_init(&dir->CurrentEntry);
	dir->CurrentIndex = 0;
}
