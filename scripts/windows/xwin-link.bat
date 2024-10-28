@echo off
Rem Wrapper script to combine LLVM's lld-link.exe with Xwin
Rem This is mostly used to pass the /winsysroot argument to the Cargo/Rust
Rem linker, because I can't get it to respect "-Clink-args=..." when building
Rem for MSVC. I suspect this is a bug in Cargo somewhere.
Rem
Rem LLVM also has a bug that fails to find the sysroot if the target machine
Rem is not specified. See: https://github.com/llvm/llvm-project/issues/54409
%CONDA_PREFIX%\Library\bin\lld-link.exe "/WINSYSROOT:%CONDA_PREFIX%\xwin" /MACHINE:x64 %*
