# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# If we are running in a Conda environment - we are probably intending to use
# Clang-Cl for everything. Fixup some paths that don't autodetect correctly.
if(DEFINED ENV{CONDA_PREFIX})
    if(NOT CMAKE_C_COMPILER)
        find_program(CMAKE_C_COMPILER NAMES clang-cl REQUIRED DOC "Clang C Compiler (MSVC Compatible)")
    endif()
    if(NOT CMAKE_CXX_COMPILER)
        find_program(CMAKE_CXX_COMPILER NAMES clang-cl REQUIRED DOC "Clang C++ Compiler (MSVC Compatible)")
    endif()
    if(NOT CMAKE_MT)
        find_program(CMAKE_MT NAMES llvm-mt REQUIRED DOC "LLVM Manifest Tool")
    endif()

    # Clear Conda's attempts to enforce optimization. It breaks on an MSVC-style compiler.
    set(ENV{AR})
    set(ENV{CC})
    set(ENV{CXX})
    set(ENV{CFLAGS})
    set(ENV{CXXFLAGS})
    set(ENV{LD})
    set(ENV{RANLIB})

    if(EXISTS $ENV{CONDA_PREFIX}/xwin)
        set(CMAKE_C_FLAGS "/winsysroot $ENV{CONDA_PREFIX}\\xwin")
        set(CMAKE_EXE_LINKER_FLAGS "/winsysroot:$ENV{CONDA_PREFIX}\\xwin")
    endif()
endif()

## CMake also has trouble finding OpenSSL libraries on Windows, and may need some help.
if(EXISTS "C:/MozillaVPNBuild/SSL" AND NOT DEFINED OPENSSL_ROOT_DIR)
    set(OPENSSL_ROOT_DIR "C:/MozillaVPNBuild/SSL")
    find_package(OpenSSL REQUIRED)
endif()
