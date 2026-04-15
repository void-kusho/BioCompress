# Phase 1: Core Implementation - Discussion Log

> **Audit trail only.** Do not use as input to planning, research, or execution agents.
> Decisions are captured in CONTEXT.md — this log preserves the alternatives considered.

**Date:** 2026-04-14
**Phase:** 1-core
**Areas discussed:** Project Structure, FASTA Parser, FASTQ Parser, Huffman Algorithm, ANS Compression, File Format

---

## Project Structure

| Option | Description | Selected |
|--------|-------------|----------|
| By language | src-c/, src-rust/, Makefile, Cargo.toml | ✓ |
| By function | src/core/, src/cli/, build/ | |
| Flat split | c/, rust/ under single src/ | |

**User's choice:** By language
**Notes:** Clear separation for C core vs Rust CLI

---

## FASTA Parser

| Option | Description | Selected |
|--------|-------------|----------|
| 128MB | Conservative, handles large genomes | |
| 512MB | Balanced | |
| Unlimited | Stream processing | ✓ |

**User's choice:** Unlimited - must handle big genomes
**Notes:** Streaming approach for large files

---

| Option | Description | Selected |
|--------|-------------|----------|
| Basic | Just extract content after '>' | ✓ |
| Standard | Parse fields separated by '|' | |
| Full | Full GFF-like annotation support | |

**User's choice:** Basic
**Notes:** Simple header extraction

---

| Option | Description | Selected |
|--------|-------------|----------|
| One at a time | Stream sequentially | ✓ |
| Buffered batch | N records in memory | |
| Full load | All into memory | |

**User's choice:** One at a time
**Notes:** Streaming for multi-record files

---

## FASTQ Parser

| Option | Description | Selected |
|--------|-------------|----------|
| Phred+33 | Standard Sanger | |
| Auto-detect | Phred+33 or +64 | |
| All encodings | Support Illumina 1.3-1.8 + Sanger | ✓ |

**User's choice:** All encodings
**Notes:** For compatibility with all FASTQ variants

---

| Option | Description | Selected |
|--------|-------------|----------|
| Strict | Only ACGTUN | ✓ |
| Lenient | IUPAC ambiguity codes | |
| None | Pass through | |

**User's choice:** Strict
**Notes:** Strict validation for quality

---

## Huffman Algorithm

| Option | Description | Selected |
|--------|-------------|----------|
| 2-bit | A=00, C=01, G=10, T=11 | ✓ |
| 4-bit | Full byte per base | |
| Extended | ACGTN + quality scores | |

**User's choice:** 2-bit
**Notes:** Space-saving approach

---

| Option | Description | Selected |
|--------|-------------|----------|
| Canonical | Faster decoding | ✓ |
| Standard | Simpler implementation | |

**User's choice:** Canonical
**Notes:** Faster decoding is important

---

## ANS Compression

| Option | Description | Selected |
|--------|-------------|----------|
| tANS | Table-based ANS | |
| rANS | Range ANS | |
| Both | Select by compression level | ✓ |

**User's choice:** Both
**Notes:** Selectable by compression level

---

| Option | Description | Selected |
|--------|-------------|----------|
| 32-bit | Standard | ✓ |
| 64-bit | For very large files | |

**User's choice:** 32-bit
**Notes:** Standard state size

---

## File Format

| Option | Description | Selected |
|--------|-------------|----------|
| Block-based | Each record is a block | |
| Stream | Full stream as one unit | |
| Hybrid | Small header + blocks | ✓ |

**User's choice:** Hybrid
**Notes:** User emphasized header must ensure quality and reliability

---

| Option | Description | Selected |
|--------|-------------|----------|
| Magic + version + checksum | Basic | ✓ |
| Magic + version + algorithm IDs + checksum + size | |
| Full metadata | includes compression level, original filename, timestamp | |

**User's choice:** Magic + version + checksum
**Notes:** For reliability and trust

---

*Phase: 01-core*
*Context gathered: 2026-04-14*