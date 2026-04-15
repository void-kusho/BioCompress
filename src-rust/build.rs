fn main() {
    cc::Build::new()
        .file("../src-c/src/main.c")
        .file("../src-c/src/compress.c")
        .file("../src-c/src/huffman.c")
        .file("../src-c/src/ans.c")
        .file("../src-c/src/io.c")
        .file("../src-c/src/fasta.c")
        .file("../src-c/src/fastq.c")
        .include("../src-c/include")
        .flag("-O3")
        .flag("-Wall")
        .flag("-Wextra")
        .compile("biocompress");
    println!("cargo:rustc-link-search=native=../src-c");
    println!("cargo:rustc-link-lib=static=biocompress");
    println!("cargo:rustc-link-lib=dylib=z");
}
