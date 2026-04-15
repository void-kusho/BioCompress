# BioCompress Research Summary

## Project Context
BioCompress is a bioinformatics compression tool for FASTA and FASTQ files using improved Huffman and ANS compression methods.
- Mixed-language implementation: C core + Rust CLI
- Target: Linux + Windows platforms
- Users: Researchers and Developers

## Domain Research (2025-2026)

### Existing Tools & Approaches
| Tool | Type | Key Feature |
|------|------|-------------|
| zDUR (2026) | Reference-free | Similarity-tree mode, high speed/ratio |
| JARVIS3 | Reference-free | GA parameter optimization, best BPS |
| Hecate | Modular | Block-wise Markov, random access |
| MFCompress | FASTA-specific | Multi-profile encoding |
| DSRC2 | FASTQ-specific | Industry-oriented |
| NAF | FASTQ | Modern zstd backend |

### Compression Algorithms in Use
- **Huffman coding**: Base-level entropy coding
- **ANS (Asymmetric Numeral Systems)**: Modern alternative to Huffman, used in ZSTD
- **LZ77**: Dictionary-based, used in DSRC, gzip
- **Burrows-Wheeler Transform**: Block reordering for better compression
- **Markov models**: Sequence prediction modeling
- **Read reordering**: Grouping similar reads (OReO framework)

### Key Insights
1. **ANS-based compression** provides better speed/ratio tradeoff than traditional Huffman
2. **Hybrid approaches** (multiple codec stages) outperform single-method tools
3. **Quality score handling** is critical for FASTQ compression
4. **Read reordering** can improve compression by 10-30%

### References
- zDUR: BMC Bioinformatics 2026
- JARVIS3: Reference-free genomic compressor
- Hecate: arXiv 2603.15390
- OReO: Bioinformatics Advances 2025