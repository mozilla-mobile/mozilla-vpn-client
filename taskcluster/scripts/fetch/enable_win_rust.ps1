# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Enables the Rust Toolchain from the Fetches/win-rust task

$REPO_ROOT_PATH =resolve-path "$PSScriptRoot/../../../"
$FETCHES_PATH =resolve-path "$REPO_ROOT_PATH/../../fetches"

# Unpack the Conda pkg
cph unpack "$FETCHES_PATH/rust.conda"
$env:CARGO_HOME="$FETCHES_PATH\rust\Library"
$env:Path="$env:Path;$env:CARGO_HOME/bin"

Write-Output "Enabled Rust toolchain"
rustc --version