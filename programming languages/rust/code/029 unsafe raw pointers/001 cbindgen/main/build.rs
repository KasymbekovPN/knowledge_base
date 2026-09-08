use cbindgen;

fn main() {
    cbindgen::Builder::new()
        .with_crate(env!("CARGO_MANIFEST_DIR"))
        .with_language(cbindgen::Language::Cxx)
        .generate()
        .unwrap()
        .write_to_file("include/rust_math_lib.h");
}
