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
if(NOT CMAKE_RC_COMPILER)
    find_program(CMAKE_RC_COMPILER NAMES llvm-rc REQUIRED DOC "LLVM Resource Compiler")
endif()
if(NOT CMAKE_MT)
    find_program(CMAKE_MT NAMES llvm-mt REQUIRED DOC "LLVM Manifest Tool")
endif()
if(NOT PYTHON_EXECUTABLE)
    find_program(PYTHON_EXECUTABLE PATHS "$ENV{CONDA_PREFIX}" NAMES python REQUIRED DOC "Python Interpreter (Conda)")
endif()

if(EXISTS $ENV{CONDA_PREFIX}/xwin)
    cmake_path(CONVERT "$ENV{CONDA_PREFIX}\\xwin" TO_CMAKE_PATH_LIST XWIN_PREFIX)

    set(CMAKE_C_FLAGS_INIT "/winsysroot ${XWIN_PREFIX} -fuse-ld=lld-link")
    set(CMAKE_CXX_FLAGS_INIT "/winsysroot ${XWIN_PREFIX} -fuse-ld=lld-link")
    set(CMAKE_RC_FLAGS_INIT "/winsysroot ${XWIN_PREFIX} -fuse-ld=lld-link")
    set(CMAKE_LINKER ${CMAKE_CURRENT_LIST_DIR}/xwin-link.bat CACHE FILEPATH "LLD Linker for the Xwin SDK")
elseif(NOT CMAKE_LINKER)
    find_program(CMAKE_LINKER NAMES lld-link REQUIRED DOC "LLD Linker (MSVC Compatible)")
endif()
