fn main() {
    cc::Build::new()
        .file("../src-c/src/main.c")
        .include("../src-c/include")
        .flag("-O3")
        .flag("-Wall")
        .flag("-Wextra")
        .compile("biocompress");
    println!("cargo:rustc-link-search=native=../src-c");
    println!("cargo:rustc-link-lib=static=biocompress");
}
