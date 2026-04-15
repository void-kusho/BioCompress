# BioCompress Project

## Overview
Fast and improved compression tools for bioinformatics data (FASTA/FASTQ files) using improved Huffman and ANS compression methods.

## Vision
Provide a reliable, high-performance compression library and CLI tool for bioinformatics researchers and developers, with configurable compression levels and cross-platform support.

## Technology Stack
- **Core compression**: C (compiled with GCC)
- **CLI interface**: Rust (using FFI to C core)
- **Build system**: Cargo + Makefile for mixed build

## Project Type
- Library + CLI tool hybrid

## Key Decisions
1. Mixed C + Rust architecture for performance-critical core with modern CLI
2. ANS + Huffman compression as foundation (not generic LZ-based)
3. Configurable profiles (speed vs ratio) as primary feature
4. Linux + Windows initial support
5. Focus on reliability (pessimistic coding approach)

## Research Summary
See `.planning/research/02-domain.md` for domain analysis.

## Requirements
See `.planning/REQUIREMENTS.md`

## Roadmap
See `.planning/ROADMAP.md`

## Configuration
See `.planning/config.json`