/**
 * IO handling - File format detection and gzip support
 * Handles both gzip-compressed and plain text per FR4.4
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <zlib.h>
#include "io.h"

// IO context structure
struct io_ctx {
    FILE* file;           // Original file handle
    gzFile gzip_file;     // Gzip decompressor (if compressed)
    uint8_t* buffer;      // Read buffer for streaming
    size_t buffer_size;
    bool is_gzip;
    char* filepath;
};

// Check if file has gzip magic bytes
static bool has_gzip_magic(FILE* f) {
    unsigned char magic[2];
    long pos = ftell(f);
    
    if (fread(magic, 1, 2, f) != 2) {
        fseek(f, pos, SEEK_SET);
        return false;
    }
    
    fseek(f, pos, SEEK_SET);
    return (magic[0] == 0x1f && magic[1] == 0x8b);
}

// Detect format from extension
static file_format_t detect_from_extension(const char* filepath) {
    const char* ext = strrchr(filepath, '.');
    if (ext == NULL) {
        return FORMAT_UNKNOWN;
    }
    
    // Check for .gz suffix first
    if (strcmp(ext, ".gz") == 0) {
        // Get the inner extension
        size_t len = ext - filepath;
        if (len > 3) {
            const char* inner_ext = filepath + len - 4;
            if (strncmp(inner_ext, ".fa", 3) == 0 || 
                strncmp(inner_ext, ".fasta", 6) == 0 ||
                strncmp(inner_ext, ".fna", 4) == 0) {
                return FORMAT_COMPRESSED;
            }
            if (strncmp(inner_ext, ".fq", 3) == 0 ||
                strncmp(inner_ext, ".fastq", 6) == 0) {
                return FORMAT_COMPRESSED;
            }
        }
        return FORMAT_COMPRESSED;
    }
    
    // Plain extensions
    if (strcmp(ext, ".fasta") == 0 || strcmp(ext, ".fa") == 0 || 
        strcmp(ext, ".fna") == 0) {
        return FORMAT_FASTA;
    }
    if (strcmp(ext, ".fastq") == 0 || strcmp(ext, ".fq") == 0) {
        return FORMAT_FASTQ;
    }
    
    return FORMAT_UNKNOWN;
}

// Detect file format per FR4.4
file_format_t biocompress_detect_format(const char* filepath) {
    if (filepath == NULL) {
        return FORMAT_UNKNOWN;
    }
    
    // First check extension
    file_format_t fmt = detect_from_extension(filepath);
    if (fmt == FORMAT_COMPRESSED || fmt == FORMAT_FASTA || fmt == FORMAT_FASTQ) {
        return fmt;
    }
    
    // Try to detect by reading file
    FILE* f = fopen(filepath, "rb");
    if (f == NULL) {
        return FORMAT_UNKNOWN;
    }
    
    // Check gzip magic bytes
    if (has_gzip_magic(f)) {
        fclose(f);
        return FORMAT_COMPRESSED;
    }
    
    // Check first character for FASTA (@) or FASTQ (@)
    int first_char = fgetc(f);
    fclose(f);
    
    if (first_char == '>') {
        return FORMAT_FASTA;
    } else if (first_char == '@') {
        return FORMAT_FASTQ;
    }
    
    return FORMAT_UNKNOWN;
}

// Open input file (handles gzip transparently per FR4.4)
io_ctx_t* biocompress_open_input(const char* filepath) {
    if (filepath == NULL) {
        return NULL;
    }
    
    io_ctx_t* ctx = (io_ctx_t*)malloc(sizeof(io_ctx_t));
    if (ctx == NULL) {
        return NULL;
    }
    
    ctx->filepath = strdup(filepath);
    ctx->buffer = NULL;
    ctx->buffer_size = 0;
    ctx->is_gzip = false;
    ctx->file = NULL;
    ctx->gzip_file = NULL;
    
    // First check for gzip
    FILE* f = fopen(filepath, "rb");
    if (f == NULL) {
        free(ctx);
        return NULL;
    }
    
    if (has_gzip_magic(f)) {
        fclose(f);
        // Open as gzip
        ctx->gzip_file = gzopen(filepath, "rb");
        if (ctx->gzip_file == NULL) {
            free(ctx);
            return NULL;
        }
        ctx->is_gzip = true;
    } else {
        // Plain file
        ctx->file = f;
    }
    
    // Allocate streaming buffer
    ctx->buffer_size = 65536;  // 64KB buffer
    ctx->buffer = (uint8_t*)malloc(ctx->buffer_size);
    if (ctx->buffer == NULL) {
        if (ctx->is_gzip) {
            gzclose(ctx->gzip_file);
        } else {
            fclose(ctx->file);
        }
        free(ctx->filepath);
        free(ctx);
        return NULL;
    }
    
    return ctx;
}

// Close input and free resources
void biocompress_close_input(io_ctx_t* ctx) {
    if (ctx != NULL) {
        if (ctx->is_gzip && ctx->gzip_file != NULL) {
            gzclose(ctx->gzip_file);
        } else if (ctx->file != NULL) {
            fclose(ctx->file);
        }
        if (ctx->buffer != NULL) {
            free(ctx->buffer);
        }
        if (ctx->filepath != NULL) {
            free(ctx->filepath);
        }
        free(ctx);
    }
}

// Read data (streaming for large files per D-03)
size_t biocompress_read(io_ctx_t* ctx, uint8_t* buffer, size_t size) {
    if (ctx == NULL || buffer == NULL || size == 0) {
        return 0;
    }
    
    if (ctx->is_gzip) {
        return gzread(ctx->gzip_file, buffer, size);
    } else {
        return fread(buffer, 1, size, ctx->file);
    }
}

// Check if input is gzip compressed
bool biocompress_is_gzip(io_ctx_t* ctx) {
    return (ctx != NULL) ? ctx->is_gzip : false;
}