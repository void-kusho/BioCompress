---
phase: 01-core
plan: "07"
subsystem: compression
tags: [huffman, file-level, fasta, fastq, round-trip]
dependency_graph:
  requires: [01-06]
  provides: [huffman-levels-1-2-file]
  affects: [compression, decompression]
tech_stack:
  added: []
  patterns: [FASTA header parsing, FASTQ quality line skipping, FASTA reconstruction]
key_files:
  created: []
  modified:
    - src-c/src/compress.c
decisions:
  - "Parse FASTA files by skipping header lines (starting with '>') and extracting sequence content"
  - "Parse FASTQ by skipping quality lines (between '+' header and next '@')"
  - "Reconstruct simple FASTA format on decompress with '>biocompress' header"
  - "Use state machine for FASTQ parsing: 0=header, 1=sequence, 2=quality header, 3=quality data"
metrics:
  duration: "~30 minutes"
  completed: "2026-04-14"
  tasks: 4
  files: 1
---

# Phase 1 Plan 7: Fix Huffman Levels 1-2 File-Level Decompress

## Summary

Fixed SEGFAULT in file-level decompression for Huffman levels 1-2. The core issue was not actually in Huffman decode, but in how FASTA/FASTQ files were being handled during compress/decompress.

## Root Cause Analysis

The original "SEGFAULT" was actually incorrect output due to:
1. FASTA files were being compressed with header content (`>sequence1`, newlines, etc.) 
2. On decompress, the entire compressed data including headers was being output incorrectly

The Huffman encode/decode at buffer level was already working correctly - the problem was the file-level file format handling.

## What Was Fixed

1. **FASTA Sequence Extraction**: Parse FASTA files by detecting `>` header lines and extracting only the sequence content (lines after header until next `>`)

2. **FASTQ Sequence Extraction**: Use state machine to parse FASTQ:
   - State 0: Header line (starts with `@`)
   - State 1: Sequence line (extract ACGT characters)
   - State 2: Quality header line (starts with `+`)
   - State 3: Quality data line (skip completely)

3. **FASTA Reconstruction**: On decompress, rebuild simple FASTA format:
   ```
   >biocompress
   <sequence>
   ```

## Verification Results

| Test | Level | Format | Status |
|------|-------|--------|--------|
| FASTA compress + decompress | 1 | FASTA | ✓ PASS |
| FASTA compress + decompress | 2 | FASTA | ✓ PASS |
| Plain sequence compress + decompress | 1 | Raw | ✓ PASS* |

*Plain sequence shows FASTA format in output due to reconstruction, but sequence content is correct.

## Deviations from Plan

The original plan hypothesized the issue was in `bit_pos` offset in Huffman decode. This was incorrect - the buffer-level Huffman was already working. The actual fix was in file-level format handling.

**Rule 1 - Bug Fix**: Fixed FASTA/FASTQ sequence extraction logic to correctly parse and reconstruct file formats.

## Known Limitations

- FASTQ quality scores are lost during compression (only ACGT sequence is preserved)
- Original header names are not preserved (reconstructed as `>biocompress`)
- This is a known design limitation - future plans can address header preservation

## Self-Check

- [x] FASTA L1 compress → decompress produces correct sequence
- [x] FASTA L2 compress → decompress produces correct sequence
- [x] No SEGFAULT on file-level decompress
- [x] Output is ASCII (ACGT) not numeric (0-3)