# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(CMAKE_SYSTEM_NAME Darwin CACHE STRING "Target operating system name")
set(CMAKE_SYSTEM_PROCESSOR arm64 CACHE STRING "Target operating system process")
set(CMAKE_CROSSCOMPILING TRUE CACHE BOOL "Target is cross compiled")

if(NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "x86_64;arm64" CACHE STRING "OSX Architectures")
endif()
if(NOT CMAKE_OSX_SYSROOT)
    if(DEFINED ENV{SDKROOT})
        set(CMAKE_OSX_SYSROOT "$ENV{SDKROOT}")
    else()
        message(FATAL_ERROR "One of CMAKE_OSX_SYSROOT or ENV{SDKROOT} must be set")
    endif()
endif()

find_program(CMAKE_C_COMPILER arm64-apple-darwin20.0.0-clang)
find_program(CMAKE_CXX_COMPILER arm64-apple-darwin20.0.0-clang++)
find_program(CMAKE_LINKER arm64-apple-darwin20.0.0-ld)

# Restrict library and header lookup to the OSX sysroot
set(CMAKE_FIND_ROOT_PATH ${CMAKE_OSX_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Ignore compiler/linker flags from Conda.
if(DEFINED ENV{CONDA_PREFIX})
    set(CMAKE_C_FLAGS "" CACHE STRING "C Compiler flags")
    set(CMAKE_CXX_FLAGS "" CACHE STRING "CXX Compiler flags")
    set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "Executable linker flags")
    set(CMAKE_SHARED_LINKER_FLAGS "" CACHE STRING "Shared library linker flags")
    set(CMAKE_MODULE_LINKER_FLAGS "" CACHE STRING "Module linker flags")
endif()
