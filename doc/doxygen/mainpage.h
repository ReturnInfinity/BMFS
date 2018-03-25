/** @mainpage
 *
 * Welcome to the developers documentation site for BMFS!
 *
 * <h3>Getting Started</h3>
 *
 * At anytime the library is to be used, the very first
 * thing to do is to initialize the disk structure.
 *
 * Initializing the disk structure requires that the seek,
 * tell, read and write methods be implemented and assigned
 * to the structure. See the @ref disk-api for
 * details. If you plan on using a file to represent a disk,
 * you can use the @ref filedisk-api to make it easier.
 *
 * Once the disk is initialized, you can create the file system
 * structure using the @ref fs-api. Be sure to assign the disk
 * to the file system structure, using @ref bmfs_set_disk, before
 * calling any of the file system functions.
 *
 * <h3>Error Checking</h3>
 *
 * The error codes used by BMFS are the same that are defined
 * in errno.h, but they're negative. So an error code that would
 * occur when creating a file that already exist would be -EEXIST.
 * Similarly, when deleting a file that doesn't exist would be
 * -ENOENT. These error codes are used across the project and
 * can be converted into a string using the standard C function,
 * strerror.
 *
 * <h3>Usage in Kernels</h3>
 *
 * The BMFS library may be used in kernel space, because it
 * does not rely on the standard library for implementation.
 * There are some functions that require the standard library,
 * but they are in a separate library 'libbmfs-stdlib.a'.
 */

