/**
 * Compression orchestrator per D-10, D-12, D-13
 * Selects algorithm based on compression level
 * Uses hybrid output format with header and checksum
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <zlib.h>
#include "compress.h"
#include "huffman.h"
#include "ans.h"
#include "parser.h"

// Simple checksum calculation (CRC32 per D-13)
static uint32_t calculate_checksum(const uint8_t* data, size_t len) {
    return crc32(0L, data, len);
}

// Buffer-based compression with level selection per D-10
int biocompress_compress_buffer(const uint8_t* input, size_t input_size,
                                uint8_t** output, size_t* output_size,
                                int compression_level) {
    if (input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (compression_level < 1 || compression_level > 5) {
        compression_level = BIOCOMPRESS_LEVEL_DEFAULT;
    }
    
    uint8_t* compressed = NULL;
    size_t compressed_size = 0;
    uint8_t algorithm = 1;
    
    // Select algorithm per D-10:
    // Level 1-2: Huffman
    // Level 3-4: tANS (default)
    // Level 5: rANS
    if (compression_level <= 2) {
        // Huffman encoding
        uint32_t frequencies[4] = {25, 25, 25, 25};  // Assume uniform
        huffman_ctx_t* hctx = huffman_build(frequencies, 4);
        if (hctx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        int ret = huffman_encode(hctx, input, input_size, &compressed, &compressed_size);
        huffman_free(hctx);
        
        if (ret != BIOCOMPRESS_OK) {
            return ret;
        }
        algorithm = 1;
        
    } else if (compression_level <= 4) {
        // tANS (default per D-10)
        ans_ctx_t* actx = ans_init(ANS_VARIANT_TANS, compression_level);
        if (actx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        int ret = ans_encode(actx, input, input_size, &compressed, &compressed_size);
        ans_free(actx);
        
        if (ret != BIOCOMPRESS_OK) {
            return ret;
        }
        algorithm = 2;
        
    } else {
        // rANS (best ratio per D-10)
        ans_ctx_t* actx = ans_init(ANS_VARIANT_RANS, compression_level);
        if (actx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        int ret = ans_encode(actx, input, input_size, &compressed, &compressed_size);
        ans_free(actx);
        
        if (ret != BIOCOMPRESS_OK) {
            return ret;
        }
        algorithm = 3;
    }
    
    // Build output with header per D-12, D-13
    // Header (12 bytes) + compressed data
    size_t total_size = sizeof(biocompress_header_t) + compressed_size;
    uint8_t* out = (uint8_t*)malloc(total_size);
    if (out == NULL) {
        free(compressed);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Fill header
    biocompress_header_t header;
    header.magic = BIOCOMPRESS_MAGIC;
    header.version = BIOCOMPRESS_VERSION;
    header.algorithm = algorithm;
    header.level = (uint8_t)compression_level;
    header.reserved = 0;
    header.checksum = calculate_checksum(compressed, compressed_size);
    
    memcpy(out, &header, sizeof(header));
    memcpy(out + sizeof(header), compressed, compressed_size);
    
    *output = out;
    *output_size = total_size;
    
    free(compressed);
    
    return BIOCOMPRESS_OK;
}

// Verify checksum per D-13
static int verify_checksum(const uint8_t* data, size_t len, uint32_t expected) {
    uint32_t actual = calculate_checksum(data, len);
    return (actual == expected) ? BIOCOMPRESS_OK : BIOCOMPRESS_ERR_CHECKSUM;
}

// Decompress buffer
int biocompress_decompress_buffer(const uint8_t* input, size_t input_size,
                                  uint8_t** output, size_t* output_size) {
    if (input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (input_size < sizeof(biocompress_header_t)) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read header
    biocompress_header_t header;
    memcpy(&header, input, sizeof(header));
    
    // Verify magic
    if (header.magic != BIOCOMPRESS_MAGIC) {
        return BIOCOMPRESS_ERR_VERSION;
    }
    
    // Verify version
    if (header.version > BIOCOMPRESS_VERSION) {
        return BIOCOMPRESS_ERR_VERSION;
    }
    
    // Get compressed data
    const uint8_t* compressed = input + sizeof(header);
    size_t compressed_size = input_size - sizeof(header);
    
    // Verify checksum per D-13
    int ret = verify_checksum(compressed, compressed_size, header.checksum);
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    // Decompress based on algorithm
    uint8_t* decompressed = NULL;
    size_t decompressed_size = 0;
    
    if (header.algorithm == 1) {
        // Huffman
        uint32_t frequencies[4] = {25, 25, 25, 25};
        huffman_ctx_t* hctx = huffman_build(frequencies, 4);
        if (hctx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        ret = huffman_decode(hctx, compressed, compressed_size, &decompressed, &decompressed_size);
        huffman_free(hctx);
        
    } else if (header.algorithm == 2 || header.algorithm == 3) {
        // tANS or rANS
        ans_variant_t variant = (header.algorithm == 2) ? ANS_VARIANT_TANS : ANS_VARIANT_RANS;
        ans_ctx_t* actx = ans_init(variant, header.level);
        if (actx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        ret = ans_decode(actx, compressed, compressed_size, &decompressed, &decompressed_size);
        ans_free(actx);
        
    } else {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    *output = decompressed;
    *output_size = decompressed_size;
    
    return BIOCOMPRESS_OK;
}

// File-level compression (simplified - reads whole file)
int biocompress_compress_file(const char* input_path, const char* output_path, int level) {
    if (input_path == NULL || output_path == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Open input file
    FILE* fin = fopen(input_path, "rb");
    if (fin == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Get file size
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read input
    uint8_t* input_data = (uint8_t*)malloc(file_size);
    if (input_data == NULL) {
        fclose(fin);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    if (fread(input_data, 1, file_size, fin) != (size_t)file_size) {
        free(input_data);
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    fclose(fin);
    
    // Compress
    uint8_t* output_data = NULL;
    size_t output_size = 0;
    
    int ret = biocompress_compress_buffer(input_data, file_size, &output_data, &output_size, level);
    free(input_data);
    
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    // Write output
    FILE* fout = fopen(output_path, "wb");
    if (fout == NULL) {
        free(output_data);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (fwrite(output_data, 1, output_size, fout) != output_size) {
        free(output_data);
        fclose(fout);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    free(output_data);
    fclose(fout);
    
    return BIOCOMPRESS_OK;
}

// File-level decompression
int biocompress_decompress_file(const char* input_path, const char* output_path) {
    if (input_path == NULL || output_path == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Open input file
    FILE* fin = fopen(input_path, "rb");
    if (fin == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Get file size
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read input
    uint8_t* input_data = (uint8_t*)malloc(file_size);
    if (input_data == NULL) {
        fclose(fin);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    if (fread(input_data, 1, file_size, fin) != (size_t)file_size) {
        free(input_data);
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    fclose(fin);
    
    // Decompress
    uint8_t* output_data = NULL;
    size_t output_size = 0;
    
    int ret = biocompress_decompress_buffer(input_data, file_size, &output_data, &output_size);
    free(input_data);
    
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    // Write output
    FILE* fout = fopen(output_path, "wb");
    if (fout == NULL) {
        free(output_data);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (fwrite(output_data, 1, output_size, fout) != output_size) {
        free(output_data);
        fclose(fout);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    free(output_data);
    fclose(fout);
    
    return BIOCOMPRESS_OK;
}