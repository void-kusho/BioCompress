#ifndef BIOCOMPRESS_H
#define BIOCOMPRESS_H

#include <stdint.h>
#include <stddef.h>

// Forward declare context structure
typedef struct biocompress_ctx biocompress_ctx;

// Compression levels
#define BIOCOMPRESS_LEVEL_FAST 1
#define BIOCOMPRESS_LEVEL_DEFAULT 3
#define BIOCOMPRESS_LEVEL_MAX 5

// Error codes
#define BIOCOMPRESS_OK 0
#define BIOCOMPRESS_ERR_INVALID_INPUT -1
#define BIOCOMPRESS_ERR_MEMORY -2
#define BIOCOMPRESS_ERR_CHECKSUM -3
#define BIOCOMPRESS_ERR_VERSION -4

/**
 * Compress input data using selected algorithm based on compression level.
 * 
 * @param input       Pointer to input data
 * @param input_size  Size of input data in bytes
 * @param output      Pointer to store allocated output buffer (caller must free)
 * @param output_size Pointer to store output size
 * @param compression_level 1-5 (1=fastest, 5=best ratio)
 * @return BIOCOMPRESS_OK on success, error code on failure
 */
int biocompress_compress(const uint8_t* input, size_t input_size,
                         uint8_t** output, size_t* output_size,
                         int compression_level);

/**
 * Decompress previously compressed data.
 * 
 * @param input       Pointer to compressed data
 * @param input_size  Size of compressed data
 * @param output      Pointer to store allocated output buffer (caller must free)
 * @param output_size Pointer to store output size
 * @return BIOCOMPRESS_OK on success, error code on failure
 */
int biocompress_decompress(const uint8_t* input, size_t input_size,
                           uint8_t** output, size_t* output_size);

/**
 * Free memory allocated by biocompress functions.
 * 
 * @param ptr Pointer to memory to free
 */
void biocompress_free(void* ptr);

/**
 * Get library version string.
 * 
 * @return Version string (do not modify or free)
 */
const char* biocompress_version(void);

/**
 * Get error message for error code.
 * 
 * @param error_code Error code from biocompress functions
 * @return Error message string (do not modify or free)
 */
const char* biocompress_error_str(int error_code);

#endif // BIOCOMPRESS_H