#ifndef BMFS_TIME_H
#define BMFS_TIME_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Gets the current time in Unix format.
 * This is used to set creation and modification
 * times on BMFS entries.
 * @param time The variable that will receive
 * the current time.
 * @returns Zero on success, an error code
 * on failure.
 * */

int bmfs_get_current_time(uint64_t *time);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_TIME_H */
