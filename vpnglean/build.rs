/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

use std::env;
use std::path::PathBuf;

use cbindgen;

const HEADER_FILE_NAME: &str = "vpnglean.h";

fn main() -> Result<(), String> {
    // Generate C++ header using cbindgen.
    let source_dir = env::var("CARGO_MANIFEST_DIR")
        .or::<String>(Ok(String::from(".")))
        .unwrap();
    let target_dir = env::var("CARGO_TARGET_DIR")
        .or::<String>(Ok(String::from("target")))
        .unwrap();
    cbindgen::generate(source_dir.clone())
        .expect("Error generating C++ headers.")
        .write_to_file(PathBuf::from(target_dir).join(HEADER_FILE_NAME));

    Ok(())
}
