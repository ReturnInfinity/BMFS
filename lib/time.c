#include <bmfs/time.h>

#include <time.h>

int bmfs_get_current_time(uint64_t *time_ptr) {

	*time_ptr = time(NULL);

	return 0;
}
