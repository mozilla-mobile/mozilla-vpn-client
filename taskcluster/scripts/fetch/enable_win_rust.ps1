# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Enables the Rust Toolchain from the Fetches/win-rust task
$RUST_VERSION = "1.59.0"


$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$FETCHES_PATH =resolve-path "$REPO_ROOT_PATH/../../fetches"
$RUST_DIR_NAME ="rust-$RUST_VERSION-x86_64-pc-windows-msvc"

$RUST_BIN=resolve-path "$FETCHES_PATH/$RUST_DIR_NAME/rustc/bin"
$CARGO_BIN=resolve-path "$FETCHES_PATH/$RUST_DIR_NAME/cargo/bin"
$RUST_LIB =resolve-path "$FETCHES_PATH/$RUST_DIR_NAME/rustc/lib" 

$env:PATH="$RUST_BIN;$CARGO_BIN;$env:PATH"
$env:LD_LIBRARY_PATH="$RUST_LIB;$env:LD_LIBRARY_PATH"

Write-Output "Enabled Rust toolchain"
rustc --version