#!/bin/sh
# Wrapper script to combine LLVM's lld-link.exe with Xwin
# This is mostly used to pass the /winsysroot argument to the Cargo/Rust
# linker, because I can't get it to respect "-Clink-args=..." when building
# for MSVC. I suspect this is a bug in Cargo somewhere.
#
# LLVM also has a bug that fails to find the sysroot if the target machine
# is not specified. See: https://github.com/llvm/llvm-project/issues/54409
${CONDA_PREFIX}/bin/lld-link "/WINSYSROOT:${CONDA_PREFIX}/xwin" /MACHINE:x64 "$@"
