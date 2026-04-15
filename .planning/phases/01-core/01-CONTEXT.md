# Phase 1: Core Implementation - Context

**Gathered:** 2026-04-14
**Status:** Ready for planning

<domain>
## Phase Boundary

Phase 1 delivers the core compression engine: C core library with FASTA/FASTQ parsing, Huffman coding, and ANS compression. This is the foundational phase - all subsequent phases depend on it.

</domain>

<decisions>
## Implementation Decisions

### Project Structure
- **D-01:** By language directory structure (src-c/ for C core, src-rust/ for Rust CLI)
- **D-02:** Build system: GNU Make + Cargo (full optimization control via Makefile)

### FASTA Parser
- **D-03:** Streaming processing (unlimited memory for large genomes)
- **D-04:** Basic header parsing (extract content after '>')
- **D-05:** Multi-record: one-at-a-time streaming

### FASTQ Parser
- **D-06:** Quality encoding: Support all (Phred+33, +64, Illumina 1.3-1.8)
- **D-07:** Sequence validation: Strict (ACGTUN only)

### Huffman Algorithm
- **D-08:** Symbol alphabet: 2-bit (A=00, C=01, G=10, T=11)
- **D-09:** Variant: Canonical (faster decoding)

### ANS Compression
- **D-10:** ANS variant: Both tANS and rANS (selectable by compression level)
- **D-11:** State word size: 32-bit

### File Format
- **D-12:** Output structure: Hybrid (small header + block-based data)
- **D-13:** Header: Magic bytes + version + checksum (for reliability)

</decisions>

<canonical_refs>
## Canonical References

**Downstream agents MUST read these before planning or implementing.**

### Project Level
- `.planning/PROJECT.md` — Vision, technology stack (C + Rust mixed)
- `.planning/REQUIREMENTS.md` — FR1.1-FR1.5, FR4.1-FR4.4, NFR1, NFR2, NFR4
- `.planning/config.json` — Preferences and workflow settings

### Domain Research
- `.planning/research/02-domain.md` — ANS benchmarks, JARVIS3, zDUR approaches

[No external specs — requirements fully captured in decisions above]

</canonical_refs>

<code_context>
## Existing Code Insights

### Project State
- **Existing code:** None yet - greenfield project
- **Directory:** Empty except for .planning/ created during project init

### Integration Points
- **C core** → `/src-c/` directory for C library
- **Rust CLI** → `/src-rust/` directory using FFI to C
- **Build** → Makefile for C (GCC flags), Cargo.toml for Rust

</code_context>

<specifics>
## Specific Ideas

[No specific references — standard approaches chosen except for streaming large genomes and all quality encodings]

</specifics>

<deferred>
## Deferred Ideas

[None — discussion stayed within phase scope]

</deferred>

---

*Phase: 01-core*
*Context gathered: 2026-04-14*