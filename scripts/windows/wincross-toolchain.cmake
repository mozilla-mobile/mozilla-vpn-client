# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Specify default system configuration
set(CMAKE_SYSTEM_NAME Windows CACHE STRING "Target operating system name")
set(CMAKE_SYSTEM_PROCESSOR x86_64 CACHE STRING "Target operating system processor")
set(CMAKE_CROSSCOMPILING TRUE CACHE BOOL "Target is cross compiled")

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
if(NOT CMAKE_LINKER)
    find_program(CMAKE_LINKER NAMES lld-link REQUIRED DOC "LLD Linker (MSVC Compatible)")
endif()
if(NOT CMAKE_MT)
    find_program(CMAKE_MT NAMES llvm-mt REQUIRED DOC "LLVM Manifest Tool")
endif()
if(NOT PYTHON_EXECUTABLE)
    find_program(PYTHON_EXECUTABLE PATHS "$ENV{CONDA_PREFIX}" NAMES python REQUIRED DOC "Python Interpreter (Conda)")
endif()

if(EXISTS $ENV{CONDA_PREFIX}/xwin)
    cmake_path(CONVERT "$ENV{CONDA_PREFIX}/xwin" TO_CMAKE_PATH_LIST XWIN_PREFIX)

    set(CMAKE_C_FLAGS_INIT "/winsysroot ${XWIN_PREFIX} -fuse-ld=lld-link -target ${CMAKE_SYSTEM_PROCESSOR}-pc-windows-msvc")
    set(CMAKE_CXX_FLAGS_INIT "/winsysroot ${XWIN_PREFIX} -fuse-ld=lld-link -target ${CMAKE_SYSTEM_PROCESSOR}-pc-windows-msvc")
    set(CMAKE_RC_FLAGS_INIT "/winsysroot ${XWIN_PREFIX} -fuse-ld=lld-link -target ${CMAKE_SYSTEM_PROCESSOR}-pc-windows-msvc")

    set(CMAKE_EXE_LINKER_FLAGS_INIT "/winsysroot:${XWIN_PREFIX}")
    set(CMAKE_MODULE_LINKER_FLAGS_INIT "/winsysroot:${XWIN_PREFIX}")
    set(CMAKE_SHARED_LINKER_FLAGS_INIT "/winsysroot:${XWIN_PREFIX}")
endif()
