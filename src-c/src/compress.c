/**
 * Compression orchestrator per D-10, D-12, D-13
 * Selects algorithm based on compression level
 * Uses hybrid output format with header and checksum
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <unistd.h>
#include <zlib.h>
#include "compress.h"
#include "huffman.h"
#include "ans.h"
#include "parser.h"

// Extract sequence from FASTA content (skip header line)
static size_t extract_fasta_sequence(const uint8_t* input, size_t input_size,
                                     const uint8_t** sequence_out) {
    // Find first newline
    const uint8_t* newline = (const uint8_t*)memchr(input, '\n', input_size);
    if (newline == NULL || input[0] != '>') {
        // Not FASTA format, return as-is
        *sequence_out = input;
        return input_size;
    }
    
    // Skip past the newline (start of sequence)
    size_t header_len = (newline - input) + 1;
    *sequence_out = input + header_len;
    return input_size - header_len;
}

// Rebuild FASTA with original header
static int rebuild_fasta(const char* original_header, size_t header_len,
                         const uint8_t* sequence, size_t seq_len,
                         uint8_t** output, size_t* output_size) {
    // Allocate: header + newline + sequence + newline
    *output_size = header_len + 1 + seq_len + 1;
    *output = (uint8_t*)malloc(*output_size);
    if (*output == NULL) {
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    memcpy(*output, original_header, header_len);
    (*output)[header_len] = '\n';
    memcpy(*output + header_len + 1, sequence, seq_len);
    (*output)[header_len + 1 + seq_len] = '\n';
    
    return BIOCOMPRESS_OK;
}

// Simple checksum calculation (CRC32 per D-13)
static uint32_t calculate_checksum(const uint8_t* data, size_t len) {
    return crc32(0L, data, len);
}

// Buffer-based compression with level selection per D-10
int biocompress_compress_buffer(const uint8_t* input, size_t input_size,
                                uint8_t** output, size_t* output_size,
                                int compression_level) {
    if (input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (compression_level < 1 || compression_level > 5) {
        compression_level = BIOCOMPRESS_LEVEL_DEFAULT;
    }
    
    uint8_t* compressed = NULL;
    size_t compressed_size = 0;
    uint8_t algorithm = 1;
    
    // Select algorithm per D-10:
    // Level 1-2: Huffman
    // Level 3-4: tANS (default)
    // Level 5: rANS
    if (compression_level <= 2) {
        // Huffman encoding
        uint32_t frequencies[4] = {25, 25, 25, 25};  // Assume uniform
        huffman_ctx_t* hctx = huffman_build(frequencies, 4);
        if (hctx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        int ret = huffman_encode(hctx, input, input_size, &compressed, &compressed_size);
        huffman_free(hctx);
        
        if (ret != BIOCOMPRESS_OK) {
            return ret;
        }
        algorithm = 1;
        
    } else if (compression_level <= 4) {
        // tANS (default per D-10)
        ans_ctx_t* actx = ans_init(ANS_VARIANT_TANS, compression_level);
        if (actx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        int ret = ans_encode(actx, input, input_size, &compressed, &compressed_size);
        ans_free(actx);
        
        if (ret != BIOCOMPRESS_OK) {
            return ret;
        }
        algorithm = 2;
        
    } else {
        // rANS (best ratio per D-10)
        ans_ctx_t* actx = ans_init(ANS_VARIANT_RANS, compression_level);
        if (actx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        int ret = ans_encode(actx, input, input_size, &compressed, &compressed_size);
        ans_free(actx);
        
        if (ret != BIOCOMPRESS_OK) {
            return ret;
        }
        algorithm = 3;
    }
    
    // Build output with header per D-12, D-13
    // Header (12 bytes) + compressed data
    size_t total_size = sizeof(biocompress_header_t) + compressed_size;
    uint8_t* out = (uint8_t*)malloc(total_size);
    if (out == NULL) {
        free(compressed);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    // Fill header
    biocompress_header_t header;
    header.magic = BIOCOMPRESS_MAGIC;
    header.version = BIOCOMPRESS_VERSION;
    header.algorithm = algorithm;
    header.level = (uint8_t)compression_level;
    header.reserved = 0;
    header.checksum = calculate_checksum(compressed, compressed_size);
    
    memcpy(out, &header, sizeof(header));
    memcpy(out + sizeof(header), compressed, compressed_size);
    
    *output = out;
    *output_size = total_size;
    
    free(compressed);
    
    return BIOCOMPRESS_OK;
}

// Verify checksum per D-13
static int verify_checksum(const uint8_t* data, size_t len, uint32_t expected) {
    uint32_t actual = calculate_checksum(data, len);
    return (actual == expected) ? BIOCOMPRESS_OK : BIOCOMPRESS_ERR_CHECKSUM;
}

// Decompress buffer
int biocompress_decompress_buffer(const uint8_t* input, size_t input_size,
                                  uint8_t** output, size_t* output_size) {
    if (input == NULL || output == NULL || output_size == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (input_size < sizeof(biocompress_header_t)) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read header
    biocompress_header_t header;
    memcpy(&header, input, sizeof(header));
    
    // Verify magic
    if (header.magic != BIOCOMPRESS_MAGIC) {
        return BIOCOMPRESS_ERR_VERSION;
    }
    
    // Verify version
    if (header.version > BIOCOMPRESS_VERSION) {
        return BIOCOMPRESS_ERR_VERSION;
    }
    
    // Get compressed data
    const uint8_t* compressed = input + sizeof(header);
    size_t compressed_size = input_size - sizeof(header);
    
    // Verify checksum per D-13
    int ret = verify_checksum(compressed, compressed_size, header.checksum);
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    // Decompress based on algorithm
    uint8_t* decompressed = NULL;
    size_t decompressed_size = 0;
    
    if (header.algorithm == 1) {
        // Huffman
        uint32_t frequencies[4] = {25, 25, 25, 25};
        huffman_ctx_t* hctx = huffman_build(frequencies, 4);
        if (hctx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        ret = huffman_decode(hctx, compressed, compressed_size, &decompressed, &decompressed_size);
        huffman_free(hctx);
        
    } else if (header.algorithm == 2 || header.algorithm == 3) {
        // tANS or rANS
        ans_variant_t variant = (header.algorithm == 2) ? ANS_VARIANT_TANS : ANS_VARIANT_RANS;
        ans_ctx_t* actx = ans_init(variant, header.level);
        if (actx == NULL) {
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        ret = ans_decode(actx, compressed, compressed_size, &decompressed, &decompressed_size);
        ans_free(actx);
        
    } else {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    *output = decompressed;
    *output_size = decompressed_size;
    
    return BIOCOMPRESS_OK;
}

// File-level compression (extracts sequence-only from FASTA/FASTQ, rebuilds on decompress)
int biocompress_compress_file(const char* input_path, const char* output_path, int level) {
    if (input_path == NULL || output_path == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Open input file
    FILE* fin = fopen(input_path, "rb");
    if (fin == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Get file size
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read input
    uint8_t* input_data = (uint8_t*)malloc(file_size);
    if (input_data == NULL) {
        fclose(fin);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    if (fread(input_data, 1, file_size, fin) != (size_t)file_size) {
        free(input_data);
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    fclose(fin);
    
    // Detect format and extract only sequence data (skip headers, newlines)
    uint8_t* sequence_data = NULL;
    size_t sequence_size = 0;
    int is_fasta = 0;
    
    // Check if FASTA format (starts with '>')
    if (file_size > 0 && input_data[0] == '>') {
        is_fasta = 1;
    }
    // Check if FASTQ format (starts with '@')
    else if (file_size > 0 && input_data[0] == '@') {
        is_fasta = 2;  // Use 2 for FASTQ
    }
    
    if (is_fasta > 0) {
        // FASTA parsing: lines starting with '>' are headers, everything else is sequence
        // FASTQ parsing: '@' is header, '+' is quality header, sequence is between
        sequence_data = (uint8_t*)malloc(file_size);
        if (sequence_data == NULL) {
            free(input_data);
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        // For FASTQ: 0=header line, 1=sequence line, 2=quality header, 3=quality data
        int fq_state = 0;
        
        for (size_t i = 0; i < (size_t)file_size; i++) {
            uint8_t c = input_data[i];
            
            if (is_fasta == 1) {
                // FASTA: '>' marks header, lines after are sequence
                if (c == '>') {
                    fq_state = 0;  // Reset to header mode
                } else if (c == '\n' || c == '\r') {
                    // End of line - if we were in a header, next line starts sequence
                    fq_state = 1;
                } else if (fq_state == 1 && c != ' ') {
                    // In sequence line - add everything
                    sequence_data[sequence_size++] = c;
                }
            } else {
                // FASTQ: '@' is header, '+' is quality header, sequence is between
                if (c == '@') {
                    fq_state = 0;  // Header line
                } else if (c == '+' && (fq_state == 1 || fq_state == 0)) {
                    // '+' quality header - appears after sequence line (fq_state=1)
                    // or after quality header (but we only care about first +)
                    fq_state = 2;
                } else if (c == '\n' || c == '\r') {
                    // End of line
                    if (fq_state == 0) {
                        // End of header line - next line is sequence
                        fq_state = 1;
                    } else if (fq_state == 2) {
                        // End of quality header line
                        fq_state = 3;
                    }
                } else if (fq_state == 1) {
                    // In sequence line - add ACGT only
                    if (c == 'A' || c == 'a' || c == 'C' || c == 'c' ||
                        c == 'G' || c == 'g' || c == 'T' || c == 't') {
                        sequence_data[sequence_size++] = c;
                    }
                }
                // fq_state == 3 means quality data, skip
            }
        }
        
        // Resize to actual size
        uint8_t* resized = (uint8_t*)realloc(sequence_data, sequence_size);
        if (resized != NULL) {
            sequence_data = resized;
        }
        
        free(input_data);
        input_data = NULL;
    } else {
        // Not FASTA/FASTQ format - treat as raw sequence
        sequence_data = (uint8_t*)malloc(file_size);
        if (sequence_data == NULL) {
            free(input_data);
            return BIOCOMPRESS_ERR_MEMORY;
        }
        
        for (size_t i = 0; i < (size_t)file_size; i++) {
            uint8_t c = input_data[i];
            if (c == 'A' || c == 'a' || c == 'C' || c == 'c' ||
                c == 'G' || c == 'g' || c == 'T' || c == 't') {
                sequence_data[sequence_size++] = c;
            }
        }
        
        uint8_t* resized = (uint8_t*)realloc(sequence_data, sequence_size);
        if (resized != NULL) {
            sequence_data = resized;
        }
        
        free(input_data);
        input_data = NULL;
    }
    
    // Compress only the sequence data
    uint8_t* output_data = NULL;
    size_t output_size = 0;
    
    int ret = biocompress_compress_buffer(sequence_data, sequence_size, &output_data, &output_size, level);
    free(sequence_data);
    
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    // Write output
    FILE* fout = fopen(output_path, "wb");
    if (fout == NULL) {
        free(output_data);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (fwrite(output_data, 1, output_size, fout) != output_size) {
        free(output_data);
        fclose(fout);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    free(output_data);
    fclose(fout);
    
    return BIOCOMPRESS_OK;
}

// File-level decompression
int biocompress_decompress_file(const char* input_path, const char* output_path) {
    if (input_path == NULL || output_path == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Open input file
    FILE* fin = fopen(input_path, "rb");
    if (fin == NULL) {
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Get file size
    fseek(fin, 0, SEEK_END);
    long file_size = ftell(fin);
    fseek(fin, 0, SEEK_SET);
    
    if (file_size <= 0) {
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    // Read input
    uint8_t* input_data = (uint8_t*)malloc(file_size);
    if (input_data == NULL) {
        fclose(fin);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    if (fread(input_data, 1, file_size, fin) != (size_t)file_size) {
        free(input_data);
        fclose(fin);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    fclose(fin);
    
    // Decompress
    uint8_t* output_data = NULL;
    size_t output_size = 0;
    
    int ret = biocompress_decompress_buffer(input_data, file_size, &output_data, &output_size);
    free(input_data);
    
    if (ret != BIOCOMPRESS_OK) {
        return ret;
    }
    
    // Rebuild FASTA format with simple header
    // Format: >biocompress\n<sequence>\n
    size_t header_len = strlen("biocompress");
    size_t fasta_size = 1 + header_len + 1 + output_size + 1;  // >header\nseq\n
    uint8_t* fasta_data = (uint8_t*)malloc(fasta_size);
    if (fasta_data == NULL) {
        free(output_data);
        return BIOCOMPRESS_ERR_MEMORY;
    }
    
    size_t pos = 0;
    fasta_data[pos++] = '>';
    memcpy(fasta_data + pos, "biocompress", header_len);
    pos += header_len;
    fasta_data[pos++] = '\n';
    memcpy(fasta_data + pos, output_data, output_size);
    pos += output_size;
    fasta_data[pos++] = '\n';
    
    free(output_data);
    output_data = fasta_data;
    output_size = fasta_size;
    
    // Write output
    FILE* fout = fopen(output_path, "wb");
    if (fout == NULL) {
        free(output_data);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    if (fwrite(output_data, 1, output_size, fout) != output_size) {
        free(output_data);
        fclose(fout);
        return BIOCOMPRESS_ERR_INVALID_INPUT;
    }
    
    free(output_data);
    fclose(fout);
    
    return BIOCOMPRESS_OK;
}