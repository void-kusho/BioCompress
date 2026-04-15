#ifndef BIOCOMPRESS_COMPRESS_H
#define BIOCOMPRESS_COMPRESS_H

#include <stddef.h>
#include <stdint.h>
#include "biocompress.h"
#include "huffman.h"
#include "ans.h"

// Compression format header per D-12, D-13
#define BIOCOMPRESS_MAGIC 0x42435A31  // "BCZA"
#define BIOCOMPRESS_VERSION 1

typedef struct {
    uint32_t magic;
    uint8_t version;
    uint8_t algorithm;  // 1=Huffman, 2=tANS, 3=rANS
    uint8_t level;
    uint8_t reserved;
    uint32_t checksum;
} biocompress_header_t;

// Compression orchestrator - selects algorithm by level per D-10
int biocompress_compress_file(const char* input_path, const char* output_path, int level);
int biocompress_decompress_file(const char* input_path, const char* output_path);

// Buffer-based compression
int biocompress_compress_buffer(const uint8_t* input, size_t input_size,
                                uint8_t** output, size_t* output_size,
                                int compression_level);

#endif // BIOCOMPRESS_COMPRESS_H