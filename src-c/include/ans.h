#ifndef BIOCOMPRESS_ANS_H
#define BIOCOMPRESS_ANS_H

#include <stdint.h>
#include <stddef.h>
#include "biocompress.h"

#define ANS_STATE_BITS 32   // 32-bit state per D-11
#define ANS_TABLE_BITS 12   // Table size = 2^12 = 4096

typedef enum {
    ANS_VARIANT_TANS,   // Table-based ANS (faster) per D-10
    ANS_VARIANT_RANS    // Range ANS (better ratio) per D-10
} ans_variant_t;

typedef struct ans_ctx ans_ctx_t;

// Initialize ANS encoder/decoder per D-10
ans_ctx_t* ans_init(ans_variant_t variant, int compression_level);

// Encode symbol sequence
int ans_encode(ans_ctx_t* ctx,
               const uint8_t* input, size_t input_size,
               uint8_t** output, size_t* output_size);

// Decode back to original
int ans_decode(ans_ctx_t* ctx,
               const uint8_t* input, size_t input_size,
               uint8_t** output, size_t* output_size);

void ans_free(ans_ctx_t* ctx);

#endif // BIOCOMPRESS_ANS_H