#ifndef BIOCOMPRESS_IO_H
#define BIOCOMPRESS_IO_H

#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include "biocompress.h"

// Forward declare parser state
struct parser_state;
typedef struct parser_state parser_state_t;

// File format types
typedef enum {
    FORMAT_UNKNOWN,
    FORMAT_FASTA,       // .fasta, .fa, .fna
    FORMAT_FASTQ,       // .fastq, .fq
    FORMAT_COMPRESSED   // .fasta.gz, .fastq.gz, etc.
} file_format_t;

// IO context for streaming
typedef struct io_ctx io_ctx_t;

// Detect file format from extension and magic bytes
file_format_t biocompress_detect_format(const char* filepath);

// Open input (handles gzip transparently per FR4.4)
io_ctx_t* biocompress_open_input(const char* filepath);
void biocompress_close_input(io_ctx_t* ctx);

// Read data (streaming for large files per D-03)
size_t biocompress_read(io_ctx_t* ctx, uint8_t* buffer, size_t size);

// Check if input is gzip compressed
bool biocompress_is_gzip(io_ctx_t* ctx);

#endif // BIOCOMPRESS_IO_H