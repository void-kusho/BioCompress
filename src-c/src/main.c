/**
 * Biocompress - Core C library
 * Main entry point and compression API implementation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "biocompress.h"

// Version string
#define BIOCOMPRESS_VERSION "0.1.0"

struct biocompress_ctx {
    int compression_level;
    // Additional context fields can be added here
};

const char* biocompress_version(void) {
    return BIOCOMPRESS_VERSION;
}

const char* biocompress_error_str(int error_code) {
    switch (error_code) {
        case BIOCOMPRESS_OK:
            return "Success";
        case BIOCOMPRESS_ERR_INVALID_INPUT:
            return "Invalid input";
        case BIOCOMPRESS_ERR_MEMORY:
            return "Memory allocation failed";
        case BIOCOMPRESS_ERR_CHECKSUM:
            return "Checksum verification failed";
        case BIOCOMPRESS_ERR_VERSION:
            return "Unsupported version";
        default:
            return "Unknown error";
    }
}

int biocompress_compress(const uint8_t* input, size_t input_size,
                        uint8_t** output, size_t* output_size,
                        int compression_level) {
    // Stub: return invalid input until implementation complete
    (void)input;
    (void)input_size;
    (void)output;
    (void)output_size;
    (void)compression_level;
    return BIOCOMPRESS_ERR_INVALID_INPUT;
}

int biocompress_decompress(const uint8_t* input, size_t input_size,
                          uint8_t** output, size_t* output_size) {
    // Stub: return invalid input until implementation complete
    (void)input;
    (void)input_size;
    (void)output;
    (void)output_size;
    return BIOCOMPRESS_ERR_INVALID_INPUT;
}

void biocompress_free(void* ptr) {
    if (ptr != NULL) {
        free(ptr);
    }
}