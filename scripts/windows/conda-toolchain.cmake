# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This file is intended to be used as a CMake toolchain file, and selects the
# clang-cl (MSVC compatible) compiler from the current conda prefix for use in
# building the application.
#
# To use this, set the CMAKE_TOOLCHAIN_FILE argument when setting up the Cmake
# project. For example:
#   cmake .. -DCMAKE_TOOLCHAIN_FILE=scripts/windows/conda-toolchain.cmake
if(NOT DEFINED ENV{CONDA_PREFIX})
    message(WARNING "Conda toolchain specified, but not in a Conda environment")
    return()
endif()

# Set the C++ compiler and tools.
if(NOT CMAKE_C_COMPILER)
    find_program(CMAKE_C_COMPILER NAMES clang-cl REQUIRED DOC "Clang C Compiler (MSVC Compatible)")
endif()
if(NOT CMAKE_CXX_COMPILER)
    find_program(CMAKE_CXX_COMPILER NAMES clang-cl REQUIRED DOC "Clang C++ Compiler (MSVC Compatible)")
endif()
if(NOT CMAKE_LINKER)
    find_program(CMAKE_LINKER NAMES lld-link REQUIRED DOC "LLD Linker (MSVC Compatible)")
endif()
if(NOT CMAKE_RC_COMPILER)
    find_program(CMAKE_RC_COMPILER NAMES llvm-rc REQUIRED DOC "LLVM Resource Compiler")
endif()
if(NOT CMAKE_MT)
    find_program(CMAKE_MT NAMES llvm-mt REQUIRED DOC "LLVM Manifest Tool")
endif()
if(NOT PYTHON_EXECUTABLE)
    find_program(PYTHON_EXECUTABLE PATHS "$ENV{CONDA_PREFIX}" NAMES python REQUIRED DOC "Python Interpreter (Conda)")
endif()

# Write a cargo config file to pass the compiler to Rust too.
function(generate_cargo_config RUST_TARGET_ARCH)
    file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/cargo_home)
    configure_file(${CMAKE_SOURCE_DIR}/scripts/windows/cargo-config.toml.in ${CMAKE_BINARY_DIR}/cargo_home/config.toml)
endfunction()
generate_cargo_config(x86_64-pc-windows-msvc)
