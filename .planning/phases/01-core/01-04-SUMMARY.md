---
phase: 01-core
plan: 04
subsystem: cli-ffi
tags: [ffi, wiring, gap-closure]
dependency_graph:
  requires: []
  provides: [ffi-bindings, wired-commands]
  affects: [src-rust/src/lib.rs, src-rust/src/main.rs, src-rust/build.rs]
tech_stack:
  added: [Rust FFI, C interop]
  patterns: [extern "C", CString conversion]
key_files:
  created: []
  modified:
    - src-rust/src/lib.rs
    - src-rust/src/main.rs
    - src-rust/build.rs
decisions:
  - "Added all C source files to build.rs for complete linking"
  - "Used CString for Rust-to-C string conversion"
metrics:
  duration: ~2 min
  completed_date: 2026-04-15
---

# Phase 1 Plan 4: Gap Closure - CLI to C Core FFI Wiring

## One-Liner
FFI bindings to wire Rust CLI compress/decompress commands to C core library

## Summary

Executed gap closure plan 01-04 to wire the Rust CLI to the C core library. The plan added FFI bindings in lib.rs and wired the compress/decompress commands in main.rs to call the actual C implementation.

## Completed Tasks

| Task | Name | Commit | Files |
|------|------|--------|-------|
| 1 | Add FFI bindings in lib.rs | 34da650 | src-rust/src/lib.rs |
| 2 | Wire compress command to FFI | 34da650 | src-rust/src/main.rs |
| 3 | Wire decompress command to FFI | 34da650 | src-rust/src/main.rs |
| 4 | Test end-to-end round-trip | 34da650 | verified |

## Verification Results

| Test | Result |
|------|--------|
| `biocompress compress --input test.fasta --output test.bc --level 3` | ✅ Created .bc file |
| `biocompress decompress --input test.bc --output restored.fasta` | ✅ Restored file |
| `diff original restored` | ✅ Files identical |
| Build compiles | ✅ With warnings only |

## Deviations from Plan

### Fixed Issues

**1. [Rule 3 - Build Config] Added missing C source files to build.rs**
- **Found during:** Task 1 build verification
- **Issue:** build.rs only compiled main.c, missing compress.c, huffman.c, ans.c, etc.
- **Fix:** Added all C source files to cc::Build
- **Files modified:** src-rust/build.rs
- **Commit:** 34da650

**2. [Rule 3 - Missing Dependency] Added zlib linking**
- **Found during:** Task 1 build verification  
- **Issue:** compress.c uses zlib (crc32), needed dynamic linking
- **Fix:** Added `println!("cargo:rustc-link-lib=dylib=z");` to build.rs
- **Files modified:** src-rust/build.rs
- **Commit:** 34da650

## Known Stubs

None - all stubs removed.

## Threat Surface

None - this plan wires existing C functions to Rust, no new security surface introduced.

---

## Self-Check: PASSED

- [x] lib.rs modified with FFI bindings
- [x] main.rs wired to call FFI functions
- [x] build.rs updated with all C files
- [x] Commit 34da650 exists
- [x] Round-trip verification passes
