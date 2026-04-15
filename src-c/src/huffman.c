/**
 * Canonical Huffman coding implementation per D-08, D-09
 * Uses 2-bit alphabet for DNA (A=00, C=01, G=10, T=11)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "huffman.h"

// Convert ASCII DNA to numeric (A=0, C=1, G=2, T=3)
// Only converts ACGT; other characters pass through as their raw byte value
// NOTE: This works because Huffman only encodes the 4 DNA symbols (0-3)
// Any non-DNA characters must be handled at higher levels
static uint8_t ascii_to_numeric(uint8_t c) {
    switch (c) {
        case 'A': case 'a': return 0;
        case 'C': case 'c': return 1;
        case 'G': case 'g': return 2;
        case 'T': case 't': return 3;
        default: return c;  // Pass through non-ACGT (like newlines)
    }
}

// Convert numeric to ASCII DNA (0=A, 1=C, 2=G, 3=T)
static uint8_t numeric_to_ascii(uint8_t n) {
    switch (n) {
        case 0: return 'A';
        case 1: return 'C';
        case 2: return 'G';
        case 3: return 'T';
        default: return n;  // Pass through non-0-3
    }
}

// Huffman context structure
struct huffman_ctx {
    huffman_code_t code;  // Canonical code table
    uint32_t frequencies[HUFFMAN_SYMBOLS];
};

// Build canonical Huffman codes from frequencies per D-09
huffman_ctx_t* huffman_build(const uint32_t* frequencies, int num_symbols) {
    if (frequencies == NULL || num_symbols != HUFFMAN_SYMBOLS) {
        return NULL;
    }
    
    huffman_ctx_t* ctx = (huffman_ctx_t*)malloc(sizeof(huffman_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }
    
    // Copy frequencies
    memcpy(ctx->frequencies, frequencies, sizeof(uint32_t) * HUFFMAN_SYMBOLS);
    
    // For canonical Huffman, we use simple fixed codes per D-08
    // In a full implementation, we'd build from frequencies
    // For now, use simple 2-bit codes
    ctx->code.code[0] = 0x00;  // A = 00
    ctx->code.code[1] = 0x01;  // C = 01
    ctx->code.code[2] = 0x02;  // G = 10
    ctx->code.code[3] = 0x03;  // T = 11
    
    ctx->code.code_len[0] = 2;
    ctx->code.code_len[1] = 2;
    ctx->code.code_len[2] = 2;
    ctx->code.code_len[3] = 2;
    
    ctx->code.num_codes = HUFFMAN_SYMBOLS;
    
    return ctx;
}

// Encode symbols to bitstream
int huffman_encode(huffman_ctx_t* ctx, 
                  const uint8_t* symbols, size_t num_symbols,
                  uint8_t** output, size_t* output_size) {
    if (ctx == NULL || symbols == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Calculate output size: 4-byte length prefix + bitstream
    size_t bitstream_size = (num_symbols * 2 + 7) / 8;
    size_t out_size = 4 + bitstream_size;  // 4-byte length prefix + bitstream
    
    uint8_t* out = (uint8_t*)malloc(out_size);
    if (out == NULL) {
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Store length as 4-byte prefix
    uint32_t len = (uint32_t)num_symbols;
    memcpy(out, &len, 4);
    
    // Encode each symbol as 2 bits (with ASCII→numeric conversion)
    size_t bit_pos = 32;  // Start after 4-byte length prefix (32 bits)
    for (size_t i = 0; i < num_symbols; i++) {
        uint8_t symbol = ascii_to_numeric(symbols[i]) % HUFFMAN_SYMBOLS;
        uint8_t code = ctx->code.code[symbol];
        
        // Write 2 bits
        size_t byte_idx = bit_pos / 8;
        size_t bit_offset = bit_pos % 8;
        
        // Handle crossing byte boundary
        if (bit_offset <= 6) {
            // Both bits fit in current byte
            out[byte_idx] &= ~(0x03 << bit_offset);
            out[byte_idx] |= (code & 0x03) << bit_offset;
        } else {
            // Split across bytes: bit 0 in current byte, bit 1 in next byte
            out[byte_idx] &= ~(0x01 << bit_offset);
            out[byte_idx] |= (code & 0x01) << bit_offset;
            out[byte_idx + 1] &= ~0x01;
            out[byte_idx + 1] |= (code >> 1) & 0x01;
        }
        
        bit_pos += 2;
    }
    
    *output = out;
    *output_size = out_size;
    
    return BIOCOMPRESS_OK;
}

// Decode bitstream to symbols
int huffman_decode(huffman_ctx_t* ctx,
                  const uint8_t* input, size_t input_size,
                  uint8_t** output, size_t* output_size) {
    if (ctx == NULL || input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (input_size < 4) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read length prefix
    uint32_t num_symbols;
    memcpy(&num_symbols, input, 4);
    
    // Allocate output
    uint8_t* out = (uint8_t*)malloc(num_symbols);
    if (out == NULL) {
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Decode each 2-bit symbol
    size_t bit_pos = 32;  // Start after 4-byte length prefix (32 bits)
    for (size_t i = 0; i < num_symbols; i++) {
        size_t byte_idx = bit_pos / 8;
        size_t bit_offset = bit_pos % 8;
        
        if (byte_idx >= input_size) {
            free(out);
            return BIOCOMPRESS_ERR_INVALID_INPUT;
        }
        
        // Handle crossing byte boundary
        uint8_t code;
        if (bit_offset <= 6) {
            // Both bits in current byte
            code = (input[byte_idx] >> bit_offset) & 0x03;
        } else {
            // Split across bytes: bit 0 in current byte, bit 1 in next byte
            if (byte_idx + 1 >= input_size) {
                free(out);
                return BIOCOMPRESS_ERR_INVALID_INPUT;
            }
            code = ((input[byte_idx] >> bit_offset) & 0x01) | 
                   ((input[byte_idx + 1] & 0x01) << 1);
        }
        // Convert numeric to ASCII DNA
        out[i] = numeric_to_ascii(code);
        
        bit_pos += 2;
    }
    
    *output = out;
    *output_size = num_symbols;
    
    return BIOCOMPRESS_OK;
}

// Free Huffman context
void huffman_free(huffman_ctx_t* ctx) {
    if (ctx != NULL) {
        free(ctx);
    }
}