#ifndef BIOCOMPRESS_PARSER_H
#define BIOCOMPRESS_PARSER_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "biocompress.h"

// Quality encoding schemes per D-06
typedef enum {
    QUAL_ENCODING_SANGER,       // Phred+33 (0-93)
    QUAL_ENCODING_SOLEXA,       // Phred+64 (5-62) - deprecated but supported
    QUAL_ENCODING_ILLUMINA_13,  // Phred+64 (0-62) Illumina 1.3-1.8
    QUAL_ENCODING_ILLUMINA_18,  // Phred+64 (0-62) Illumina 1.8+
    QUAL_ENCODING_AUTO          // Auto-detect from data
} quality_encoding_t;

// FASTA record structure
typedef struct {
    char* header;       // NULL-terminated header line (without >)
    char* sequence;    // NULL-terminated sequence (ACGTUN only)
    size_t seq_len;
} fasta_record_t;

// FASTQ record structure
typedef struct {
    char* header;
    char* sequence;
    char* quality;      // Quality scores as string (ASCII)
    size_t seq_len;
    quality_encoding_t encoding;
} fastq_record_t;

// Parser state for streaming per D-05 (full definition)
struct parser_state {
    FILE* file;
    char* buffer;
    size_t buffer_size;
    bool is_fasta;        // true for FASTA, false for FASTQ
};

typedef struct parser_state parser_state_t;

// FASTA parser functions (streaming per D-03, D-05)
parser_state_t* fasta_open(const char* filepath);
void fasta_close(parser_state_t* ctx);
int fasta_next_record(parser_state_t* ctx, fasta_record_t* record);
void fasta_record_free(fasta_record_t* record);

// FASTQ parser functions with quality encoding per D-06
parser_state_t* fastq_open(const char* filepath);
void fastq_close(parser_state_t* ctx);
int fastq_next_record(parser_state_t* ctx, fastq_record_t* record);
void fastq_record_free(fastq_record_t* record);

// Validation per D-07
bool biocompress_validate_sequence(const char* seq, size_t len);

// Quality encoding detection and decoding per D-06
int biocompress_detect_quality_encoding(const char* quality, size_t len);
int biocompress_decode_quality(const char* quality, size_t len,
                               int8_t* output, quality_encoding_t encoding);

#endif // BIOCOMPRESS_PARSER_H