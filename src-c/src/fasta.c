/**
 * FASTA streaming parser implementation
 * Supports multi-line records, streaming per D-03, D-05
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "parser.h"

// Maximum line length for parsing
#define MAX_LINE_LENGTH 10240

// Validate sequence characters per D-07 (ACGTUN only)
bool biocompress_validate_sequence(const char* seq, size_t len) {
    if (seq == NULL || len == 0) {
        return false;
    }
    
    for (size_t i = 0; i < len; i++) {
        char c = seq[i];
        // Accept A, C, G, T, U, N (upper and lower case)
        if (c != 'A' && c != 'a' && c != 'C' && c != 'c' &&
            c != 'G' && c != 'g' && c != 'T' && c != 't' &&
            c != 'U' && c != 'u' && c != 'N' && c != 'n') {
            return false;
        }
    }
    return true;
}

// Normalize sequence (convert to uppercase, U to T)
static void normalize_sequence(char* seq, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (seq[i] == 'u' || seq[i] == 'U') {
            seq[i] = 'T';
        } else if (seq[i] >= 'a' && seq[i] <= 'z') {
            seq[i] = seq[i] - 'a' + 'A';
        }
    }
}

// Open a FASTA file for streaming
parser_state_t* fasta_open(const char* filepath) {
    if (filepath == NULL) {
        return NULL;
    }
    
    FILE* f = fopen(filepath, "r");
    if (f == NULL) {
        return NULL;
    }
    
    // Check first character is '>'
    int first_char = fgetc(f);
    if (first_char != '>') {
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
    state->is_fasta = true;
    
    return state;
}

// Close FASTA parser and free resources
void fasta_close(parser_state_t* ctx) {
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

// Read next FASTA record (streaming one-at-a-time per D-05)
int fasta_next_record(parser_state_t* ctx, fasta_record_t* record) {
    if (ctx == NULL || ctx->file == NULL || record == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    char line[MAX_LINE_LENGTH];
    
    // Initialize record to NULL
    record->header = NULL;
    record->sequence = NULL;
    record->seq_len = 0;
    
    // Skip to next header line (starting with '>')
    while (fgets(line, sizeof(line), ctx->file) != NULL) {
        if (line[0] == '>') {
            break;
        }
    }
    
    // Check for EOF
    if (feof(ctx->file)) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Parse header (remove '>' and trailing newline)
    size_t header_len = strlen(line);
    if (header_len > 0 && line[header_len - 1] == '\n') {
        line[header_len - 1] = '\0';
        header_len--;
    }
    if (header_len > 0 && line[header_len - 1] == '\r') {
        line[header_len - 1] = '\0';
        header_len--;
    }
    
    // Skip '>' character
    char* header_content = line + 1;
    record->header = strdup(header_content);
    if (record->header == NULL) {
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Accumulate sequence lines until next '>' or EOF
    size_t seq_capacity = 1024;
    record->sequence = (char*)malloc(seq_capacity);
    if (record->sequence == NULL) {
        free(record->header);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    record->seq_len = 0;
    
    while (fgets(line, sizeof(line), ctx->file) != NULL) {
        // Check for end of record
        if (line[0] == '>' || line[0] == '\n') {
            if (line[0] == '>') {
                // Put back the '>' for next record
                ungetc('>', ctx->file);
            }
            break;
        }
        
        // Remove trailing newline
        size_t line_len = strlen(line);
        if (line_len > 0 && line[line_len - 1] == '\n') {
            line[line_len - 1] = '\0';
            line_len--;
        }
        if (line_len > 0 && line[line_len - 1] == '\r') {
            line[line_len - 1] = '\0';
            line_len--;
        }
        
        // Expand buffer if needed
        if (record->seq_len + line_len + 1 > seq_capacity) {
            seq_capacity *= 2;
            char* new_buf = (char*)realloc(record->sequence, seq_capacity);
            if (new_buf == NULL) {
                free(record->header);
                free(record->sequence);
                return BIOCOMPRESS_ERR_MEMORY;
            }
            record->sequence = new_buf;
        }
        
        // Append to sequence
        memcpy(record->sequence + record->seq_len, line, line_len);
        record->seq_len += line_len;
    }
    
    // Null-terminate the sequence
    record->sequence[record->seq_len] = '\0';
    
    // Normalize sequence (uppercase, U -> T)
    normalize_sequence(record->sequence, record->seq_len);
    
    // Validate sequence per D-07
    if (!biocompress_validate_sequence(record->sequence, record->seq_len)) {
        free(record->header);
        free(record->sequence);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    return BIOCOMPRESS_OK;
}

// Free FASTA record resources
void fasta_record_free(fasta_record_t* record) {
    if (record != NULL) {
        if (record->header != NULL) {
            free(record->header);
        }
        if (record->sequence != NULL) {
            free(record->sequence);
        }
        record->header = NULL;
        record->sequence = NULL;
        record->seq_len = 0;
    }
}