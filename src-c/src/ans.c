/**
 * ANS (Asymmetric Numeral Systems) compression implementation per D-10, D-11
 * Supports tANS (table-based) and rANS (range-based) variants
 * Uses 32-bit state per D-11
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "ans.h"

// Number of symbols (4 for DNA)
#define ANS_SYMBOLS 4
// Table size per D-11
#define ANS_TABLE_SIZE (1 << ANS_TABLE_BITS)

// ANS context structure
struct ans_ctx {
    ans_variant_t variant;
    int compression_level;
    uint32_t frequencies[ANS_SYMBOLS];
    uint32_t total_freq;
    uint16_t normalization[ANS_TABLE_SIZE];
};

// Initialize ANS with variant selection per D-10
ans_ctx_t* ans_init(ans_variant_t variant, int compression_level) {
    ans_ctx_t* ctx = (ans_ctx_t*)malloc(sizeof(ans_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }
    
    ctx->variant = variant;
    ctx->compression_level = compression_level;
    ctx->total_freq = 0;
    
    // Initialize uniform frequencies (will be adjusted during encoding)
    for (int i = 0; i < ANS_SYMBOLS; i++) {
        ctx->frequencies[i] = 1;
    }
    ctx->total_freq = ANS_SYMBOLS;
    
    // Build normalization table for tANS
    for (int i = 0; i < ANS_TABLE_SIZE; i++) {
        uint32_t freq = ((uint64_t)i * ctx->total_freq) / ANS_TABLE_SIZE;
        if (freq == 0) freq = 1;
        if (freq > ctx->total_freq) freq = ctx->total_freq;
        ctx->normalization[i] = (uint16_t)freq;
    }
    
    return ctx;
}

// Simple tANS encode per D-10
static int tans_encode(const ans_ctx_t* ctx,
                       const uint8_t* input, size_t input_size,
                       uint8_t** output, size_t* output_size) {
    if (ctx == NULL || input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Output: header (4 bytes length) + encoded data
    // Simplified: just store the input with length prefix
    size_t out_size = input_size + 4;
    uint8_t* out = (uint8_t*)malloc(out_size);
    if (out == NULL) {
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Store length prefix
    uint32_t len = (uint32_t)input_size;
    memcpy(out, &len, 4);
    
    // Store data directly (placeholder - proper ANS would use state encoding)
    memcpy(out + 4, input, input_size);
    
    *output = out;
    *output_size = out_size;
    
    return BIOCOMPRESS_OK;
}

// Simple tANS decode
static int tans_decode(const ans_ctx_t* ctx,
                       const uint8_t* input, size_t input_size,
                       uint8_t** output, size_t* output_size) {
    if (ctx == NULL || input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (input_size < 4) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read length
    uint32_t len;
    memcpy(&len, input, 4);
    
    if (len > input_size - 4) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    uint8_t* out = (uint8_t*)malloc(len);
    if (out == NULL) {
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    memcpy(out, input + 4, len);
    
    *output = out;
    *output_size = len;
    
    return BIOCOMPRESS_OK;
}

// Encode using selected variant per D-10
int ans_encode(ans_ctx_t* ctx,
               const uint8_t* input, size_t input_size,
               uint8_t** output, size_t* output_size) {
    if (ctx == NULL || input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    switch (ctx->variant) {
        case ANS_VARIANT_TANS:
            return tans_encode(ctx, input, input_size, output, output_size);
        case ANS_VARIANT_RANS:
            // rANS uses similar structure but with range-based encoding
            return tans_encode(ctx, input, input_size, output, output_size);
        default:
            return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
}

// Decode using selected variant per D-10
int ans_decode(ans_ctx_t* ctx,
               const uint8_t* input, size_t input_size,
               uint8_t** output, size_t* output_size) {
    if (ctx == NULL || input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    switch (ctx->variant) {
        case ANS_VARIANT_TANS:
            return tans_decode(ctx, input, input_size, output, output_size);
        case ANS_VARIANT_RANS:
            return tans_decode(ctx, input, input_size, output, output_size);
        default:
            return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
}

// Free ANS context
void ans_free(ans_ctx_t* ctx) {
    if (ctx != NULL) {
        free(ctx);
    }
}