/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::env;
use std::path::PathBuf;

const HEADER_FILE_NAME: &str = "obfuscators.h";

fn main() -> Result<(), String> {
    let source_dir = env::var("CARGO_MANIFEST_DIR").unwrap_or_else(|_| ".".into());
    let target_dir = env::var("CARGO_TARGET_DIR").unwrap_or_else(|_| "target".into());

    cbindgen::generate(source_dir)
        .expect("Error generating C++ headers for obfuscators crate.")
        .write_to_file(PathBuf::from(target_dir).join(HEADER_FILE_NAME));

    Ok(())
}
