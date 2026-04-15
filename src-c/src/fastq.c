/**
 * FASTQ parser implementation
 * Supports all quality encodings per D-06
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

#define MAX_LINE_LENGTH 10240

// Detect quality encoding per D-06
int biocompress_detect_quality_encoding(const char* quality, size_t len) {
    if (quality == NULL || len == 0) {
        return QUAL_ENCODING_AUTO;
    }
    
    // Find range of quality characters
    int min_val = 127;
    int max_val = 0;
    
    for (size_t i = 0; i < len; i++) {
        int val = (int)(unsigned char)quality[i];
        if (val < min_val) min_val = val;
        if (val > max_val) max_val = val;
    }
    
    // Per D-06 encoding detection:
    // SANGER: Phred+33 (ASCII 33-126, values 0-93)
    // SOLEXA: Phred+64 (ASCII 59-111, values 5-62) - deprecated
    // Illumina 1.3-1.8: Phred+64 (ASCII 64-111, values 0-62)
    // Illumina 1.8+: Phred+64 (ASCII 66-126, values 0-93)
    
    if (max_val <= 93 && min_val >= 33) {
        return QUAL_ENCODING_SANGER;
    } else if (max_val <= 62 && min_val >= 59) {
        return QUAL_ENCODING_SOLEXA;
    } else if (max_val <= 62 && min_val >= 64) {
        return QUAL_ENCODING_ILLUMINA_13;
    } else if (max_val <= 126 && min_val >= 66) {
        return QUAL_ENCODING_ILLUMINA_18;
    }
    
    // Default to Illumina 1.8+ for unknown
    return QUAL_ENCODING_ILLUMINA_18;
}

// Decode quality string to integer Phred scores per D-06
int biocompress_decode_quality(const char* quality, size_t len,
                               int8_t* output, quality_encoding_t encoding) {
    if (quality == NULL || output == NULL || len == 0) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Auto-detect encoding if requested
    if (encoding == QUAL_ENCODING_AUTO) {
        encoding = (quality_encoding_t)biocompress_detect_quality_encoding(quality, len);
    }
    
    // Determine offset based on encoding
    int offset;
    switch (encoding) {
        case QUAL_ENCODING_SANGER:
            offset = 33;
            break;
        case QUAL_ENCODING_SOLEXA:
            offset = 64;
            break;
        case QUAL_ENCODING_ILLUMINA_13:
            offset = 64;
            break;
        case QUAL_ENCODING_ILLUMINA_18:
            offset = 64;
            break;
        default:
            offset = 33;  // Default to SANGER
    }
    
    // Decode each quality character
    for (size_t i = 0; i < len; i++) {
        int8_t score = (int8_t)((int)(unsigned char)quality[i] - offset);
        
        // Clamp to valid range (0-93 for Phred)
        if (score < 0) score = 0;
        if (score > 93) score = 93;
        
        output[i] = score;
    }
    
    return BIOCOMPRESS_OK;
}

// Open a FASTQ file for parsing
parser_state_t* fastq_open(const char* filepath) {
    if (filepath == NULL) {
        return NULL;
    }
    
    FILE* f = fopen(filepath, "r");
    if (f == NULL) {
        return NULL;
    }
    
    // Check first character is '@'
    int first_char = fgetc(f);
    if (first_char != '@') {
        fclose(f);
        return NULL;
    }
    ungetc(first_char, f);
    
    parser_state_t* state = (parser_state_t*)malloc(sizeof(parser_state_t));
    if (state == NULL) {
        fclose(f);
        return NULL;
    }
    
    state->file = f;
    state->buffer = NULL;
    state->buffer_size = 0;
    state->is_fasta = false;
    
    return state;
}

// Close FASTQ parser
void fastq_close(parser_state_t* ctx) {
    if (ctx != NULL) {
        if (ctx->file != NULL) {
            fclose(ctx->file);
        }
        if (ctx->buffer != NULL) {
            free(ctx->buffer);
        }
        free(ctx);
    }
}

// Read next FASTQ record
int fastq_next_record(parser_state_t* ctx, fastq_record_t* record) {
    if (ctx == NULL || ctx->file == NULL || record == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    char line[MAX_LINE_LENGTH];
    
    // Initialize record
    record->header = NULL;
    record->sequence = NULL;
    record->quality = NULL;
    record->seq_len = 0;
    record->encoding = QUAL_ENCODING_AUTO;
    
    // Read @header line
    if (fgets(line, sizeof(line), ctx->file) == NULL || line[0] != '@') {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Parse header
    size_t header_len = strlen(line);
    if (header_len > 0 && line[header_len - 1] == '\n') {
        line[header_len - 1] = '\0';
        header_len--;
    }
    if (header_len > 0 && line[header_len - 1] == '\r') {
        line[header_len - 1] = '\0';
        header_len--;
    }
    
    record->header = strdup(line + 1);
    if (record->header == NULL) {
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Read sequence line
    if (fgets(line, sizeof(line), ctx->file) == NULL) {
        free(record->header);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    size_t line_len = strlen(line);
    if (line_len > 0 && line[line_len - 1] == '\n') {
        line[line_len - 1] = '\0';
        line_len--;
    }
    if (line_len > 0 && line[line_len - 1] == '\r') {
        line[line_len - 1] = '\0';
        line_len--;
    }
    
    record->sequence = strdup(line);
    if (record->sequence == NULL) {
        free(record->header);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    record->seq_len = line_len;
    
    // Normalize sequence (uppercase, U -> T)
    for (size_t i = 0; i < record->seq_len; i++) {
        if (record->sequence[i] == 'u' || record->sequence[i] == 'U') {
            record->sequence[i] = 'T';
        } else if (record->sequence[i] >= 'a' && record->sequence[i] <= 'z') {
            record->sequence[i] = record->sequence[i] - 'a' + 'A';
        }
    }
    
    // Validate sequence per D-07
    if (!biocompress_validate_sequence(record->sequence, record->seq_len)) {
        free(record->header);
        free(record->sequence);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read + separator line
    if (fgets(line, sizeof(line), ctx->file) == NULL || line[0] != '+') {
        free(record->header);
        free(record->sequence);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read quality line
    if (fgets(line, sizeof(line), ctx->file) == NULL) {
        free(record->header);
        free(record->sequence);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    line_len = strlen(line);
    if (line_len > 0 && line[line_len - 1] == '\n') {
        line[line_len - 1] = '\0';
        line_len--;
    }
    if (line_len > 0 && line[line_len - 1] == '\r') {
        line[line_len - 1] = '\0';
        line_len--;
    }
    
    // Check sequence/quality length match per D-07
    if (line_len != record->seq_len) {
        free(record->header);
        free(record->sequence);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    record->quality = strdup(line);
    if (record->quality == NULL) {
        free(record->header);
        free(record->sequence);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Detect quality encoding
    record->encoding = (quality_encoding_t)biocompress_detect_quality_encoding(
        record->quality, record->seq_len);
    
    return BIOCOMPRESS_OK;
}

// Free FASTQ record resources
void fastq_record_free(fastq_record_t* record) {
    if (record != NULL) {
        if (record->header != NULL) {
            free(record->header);
        }
        if (record->sequence != NULL) {
            free(record->sequence);
        }
        if (record->quality != NULL) {
            free(record->quality);
        }
        record->header = NULL;
        record->sequence = NULL;
        record->quality = NULL;
        record->seq_len = 0;
    }
}