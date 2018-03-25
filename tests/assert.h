#ifndef BMFS_ASSERT_H
#define BMFS_ASSERT_H

#include <stdio.h>

#ifndef bmfs_assert
#define bmfs_assert(expr)                                                                     \
	do {                                                                                  \
		if (!(expr))                                                                     \
		{                                                                             \
			printf("%s:%u: assertion failed '%s'.\n", __FILE__, __LINE__, #expr); \
			abort();                                                              \
		}                                                                             \
	} while (0)
#endif /* bmfs_assert */

#endif /* BMFS_ASSERT_H */
