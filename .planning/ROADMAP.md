# BioCompress Roadmap

## Project: BioCompress - Bioinformatics Compression Tool

### Phase 1: Core Implementation
**Duration**: 3-5 days  
**Depends on**: None
**Status**: Planned (3 plans)

**Plans:**
- [x] 01-01-PLAN.md — Project structure setup
- [x] 01-02-PLAN.md — FASTA/FASTQ parsers
- [x] 01-03-PLAN.md — Huffman and ANS compression

- 1.1 Project structure setup (C core + Rust CLI mixed build)
- 1.2 FASTA parser implementation
- 1.3 FASTQ parser implementation
- 1.4 Improved Huffman coding algorithm
- 1.5 ANS compression implementation
- 1.6 File I/O handling

### Phase 2: CLI Interface
**Duration**: 2-3 days  
**Depends on**: 1

- 2.1 Rust CLI wrapper for C core
- 2.2 Command-line interface (compress/decompress commands)
- 2.3 Compression level options
- 2.4 Statistics output
- 2.5 Help and version commands

### Phase 3: Library & Integration
**Duration**: 2-3 days  
**Depends on**: 2

- 3.1 C library API documentation
- 3.2 Header file organization
- 3.3 Usage examples
- 3.4 Build system for library distribution

### Phase 4: Testing & Verification
**Duration**: 2-3 days  
**Depends on**: 3

- 4.1 Unit tests for core algorithms
- 4.2 Integration tests with real datasets
- 4.3 Compression ratio benchmarking
- 4.4 Performance profiling

### Phase 5: Polish & Release
**Duration**: 1-2 days  
**Depends on**: 4

- 5.1 Error handling review
- 5.2 Windows build verification
- 5.3 Release binary distribution
- 5.4 README and documentation

---

## Phase Dependencies
```
Phase 1 (Core) → Phase 2 (CLI) → Phase 3 (Library) → Phase 4 (Testing) → Phase 5 (Release)
```

## Parallel Tracks
- **Core compression**: Phases 1.1-1.6 (sequential)
- **CLI development**: Phase 2 (after Phase 1 complete)
- **Library packaging**: Phase 3 (after Phase 2 complete)