#!/bin/bash
set -e

BASE_PREFIX=$(conda info --base)
if [[ "$CONDA_PREFIX" == "$BASE_PREFIX" ]]; then
    echo "Please run this script inside a non-base conda environment."
    exit 1
fi

## In our .env we ship a lot of stuff that we do not need
## Let's remove that. 

## Also saves 1gb
conda remove rust-std-aarch64-apple-darwin rust-std-x86_64-apple-darwin rust-std-aarch64-apple-ios rust-std-x86_64-apple-ios --offline -y
## Saves another 1gb
sdkmanager --uninstall --sdk_root=$ANDROID_HOME emulator
## No Need for other Compilers, as we use NDK. - easy 500mb save. 
conda remove --offline -y clang clang-tools clangxx compiler-rt
## Clean the PKG cache, cleans 7gb ~~

conda clean -a -y 
