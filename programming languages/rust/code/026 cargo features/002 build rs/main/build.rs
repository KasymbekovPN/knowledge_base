use std::env;
use std::fs;
use std::path::Path;
use std::process::Command;

fn main() {
    // === Часть 1: компилируем и линкуем C-код через крейт `cc` ===
    // Аналог add_library(fast_math STATIC vendor/fast_math.c) в CMake
    // создаёт libfast_math.a и сам добавляет cargo:rustc-link-lib
    cc::Build::new()
        .file("vendor/fast_math.c")
        .compile("fast_math");
    println!("cargo:rerun-if-changed=vendor/fast_math.c");
    println!("cargo:rerun-if-changed=build.rs");

    // === Часть 2: генерируем Rust-код на этапе сборки ===
    let out_dir = env::var("OUT_DIR").unwrap();
    let dest_path = Path::new(&out_dir).join("build_info.rs");

    let git_hash = Command::new("git")
        .args(&["rev-parse", "--short", "HEAD"])
        .output()
        .ok()
        .and_then(|o| String::from_utf8(o.stdout).ok())
        .unwrap_or_else(|| "unknown".to_string())
        .trim().to_string();

    // "debug" или "release"
    let profile = env::var("PROFILE").unwrap();

    fs::write(
        &dest_path,
        format!(
            r#"
            pub const GIT_HASH: &str = "{git_hash}";
            pub const BUILD_PROFILE: &str = "{profile}";
            pub const TARGET_OS: &str = "{}";
            "#,
            env::var("CARGO_CFG_TARGET_OS").unwrap()
        ),
    ).unwrap();
}
