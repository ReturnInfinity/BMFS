/* ===============================================================
 * Baremetal File System - A file system designed for BareMetal OS
 * Copyright (C) 2008 - 2018 Return Infinity
 * See COPYING for license information.
 * ===============================================================
 */

#ifndef BMFS_CIPHER_H
#define BMFS_CIPHER_H

#ifdef __cplusplus
extern "C" {
#endif

/** @defgroup cipher-api Cipher API
 * Used for binding a cipher to the file system.
 * */

/** Information regarding the cipher algorithm
 * that is needed by the file system implementation.
 * @ingroup cipher-api
 * */

struct BMFSCipherInfo {
	/** The number of bytes for each block. */
	bmfs_uint64 block_size;
	/** The size of the cipher key. */
	bmfs_uint64 key_size;
};

/** An initialization vector of the cipher
 * block. This should be a randomly generated
 * buffer, the size of one block.
 * @ingroup cipher-api
 * */

struct BMFSCipherIV {
	/** The beginning of the initialization buffer. */
	const void *iv;
	/** The number of bytes in the initialization buffer.
	 * This should be the size of one block. */
	bmfs_uint64 iv_size;
};

/** This is the key structure, used to encrypt
 * and decrypt blocks of data.
 * @ingroup cipher-api
 * */

struct BMFSCipherKey {
	/** The beginning of the key buffer. */
	const void *key;
	/** The size of the key buffer. This
	 * should be the size given by @ref BMFSCipherInfo. */
	bmfs_uint64 key_size;
};

/** Used to pass data to the cipher algorithm,
 * to either be encrypted or decrypted.
 * @ingroup cipher-api
 * */

struct BMFSCipherBuffer {
	/** The beginning of the data that is going
	 * to be either encrypted or decrypted. */
	void *data;
	/** The number of bytes in the buffer. This
	 * should be equal to the block size of the
	 * cipher algorithm. */
	bmfs_uint64 data_size;
};

/** Used to encrypt and decrypt data on the file system.
 * This is defined by the caller.
 * @ingroup cipher-api
 * */

struct BMFSCipher {
	/** Cipher implementation data goes here. */
	void *CipherPtr;
	/** Decrypts data */
	int (*Decrypt)(void *cipher_ptr, struct BMFSCipherBuffer *buffer);
	/** Encrypts data */
	int (*Encrypt)(void *cipher_ptr, struct BMFSCipherBuffer *buffer);
	/** Get information regarding the cipher algorithm. */
	int (*GetInfo)(void *cipher_ptr, struct BMFSCipherInfo *info);
	/** Set the key used to encrypt and decrypt data. */
	int (*SetKey)(void *cipher_ptr, const struct BMFSCipherKey *key);
	/** The the initialization vector for encryption. */
	int (*SetIV)(void *cipher_ptr, const struct BMFSCipherIV *iv);
};

/** Initializes the cipher algorithm.
 * @param cipher The cipher structure to initialize.
 * @ingroup cipher-api
 * */

void bmfs_cipher_init(struct BMFSCipher *cipher);

/** Decrypt data using the cipher algorithm.
 * @param cipher An initialized cipher structure.
 * @param buffer The buffer to decrypt.
 * @returns Zero on success, an error code on failure.
 * @ingroup cipher-api
 * */

int bmfs_cipher_decrypt(struct BMFSCipher *cipher,
                        struct BMFSBuffer *buffer);

/** Encrypt data using the cipher algorithm.
 * @param cipher An initialized cipher structure.
 * @param buffer The buffer to encrypt.
 * @returns Zero on success, an error code on failure.
 * @ingroup cipher-api
 * */

int bmfs_cipher_encrypt(struct BMFSCipher *cipher,
                        struct BMFSBuffer *buffer);

/** Get information regarding the cipher algorithm.
 * @param cipher An initialized cipher structure.
 * @param info The structure to receive the cipher information.
 * @returns Zero on success, an error code on failure.
 * @ingroup cipher-api
 * */

int bmfs_cipher_get_info(struct BMFSCipher *cipher,
                         struct BMFSCipherInfo *info);

/** Set the initialization vector for when data is encrypted.
 * @param cipher An initialized cipher structure.
 * @param iv The initialization vector to use.
 * @returns Zero on success, an error code on failure.
 * @ingroup cipher-api
 * */

int bmfs_cipher_set_iv(struct BMFSCipher *cipher,
                       const struct BMFSCipherIV *iv);

/** Set the key to use when encrypting or decrypting buffers.
 * @param cipher An initialized cipher structure.
 * @param key The key to use for the cipher algorithm.
 * @returns Zero on success, an error code on failure.
 * @ingroup cipher-api
 * */

int bmfs_cipher_set_key(struct BMFSCipher *cipher,
                        const struct BMFSCipherKey *key);

#ifdef __cplusplus
} /* extern "C" { */
#endif

#endif /* BMFS_CIPHER_H */
