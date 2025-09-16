use std::path::Path;

fn main() {
    // Tell cargo to look for shared libraries in the specified directories
    let lib_path = "../../build";
    if Path::new(lib_path).exists() {
        println!("cargo:rustc-link-search={}", lib_path);
    }
    
    // Tell cargo to tell rustc to link the dice library
    println!("cargo:rustc-link-lib=dice");
    
    // Rerun if the C library changes
    println!("cargo:rerun-if-changed=../../src/dice.c");
    println!("cargo:rerun-if-changed=../../include/dice.h");
}