#ifndef BIOCOMPRESS_HUFFMAN_H
#define BIOCOMPRESS_HUFFMAN_H

#include <stdint.h>
#include <stddef.h>
#include "biocompress.h"

#define HUFFMAN_SYMBOLS 4  // A, C, G, T (2-bit per D-08)

// Canonical Huffman tree for DNA per D-08
typedef struct {
    uint8_t code[HUFFMAN_SYMBOLS];       // Canonical codes
    uint8_t code_len[HUFFMAN_SYMBOLS];   // Code lengths (1-8 bits)
    uint16_t num_codes;                   // Number of codes
} huffman_code_t;

typedef struct huffman_ctx huffman_ctx_t;

// Build canonical Huffman code from frequencies per D-09
huffman_ctx_t* huffman_build(const uint32_t* frequencies, int num_symbols);

// Encode symbol sequence to bitstream
int huffman_encode(huffman_ctx_t* ctx, 
                  const uint8_t* symbols, size_t num_symbols,
                  uint8_t** output, size_t* output_size);

// Decode bitstream to symbol sequence
int huffman_decode(huffman_ctx_t* ctx,
                  const uint8_t* input, size_t input_size,
                  uint8_t** output, size_t* output_size);

void huffman_free(huffman_ctx_t* ctx);

#endif // BIOCOMPRESS_HUFFMAN_H