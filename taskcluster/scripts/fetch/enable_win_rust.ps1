# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Enables the Rust Toolchain from the Fetches/win-rust task
$RUST_VERSION = "1.59.0"


$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$FETCHES_PATH =resolve-path "$REPO_ROOT_PATH/../../fetches"
$BUILD_DIR_PATH =resolve-path "$REPO_ROOT_PATH/../../build"
$env:CARGO_HOME="$BUILD_DIR_PATH\cargo\"

. "$FETCHES_PATH\rustup-init.exe" -y

$env:Path="$env:Path;$env:CARGO_HOME/bin"


Write-Output "Enabled Rust toolchain"
rustc --version