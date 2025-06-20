# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# Specify default system configuration
set(CMAKE_SYSTEM_NAME Darwin CACHE STRING "Target operating system name")
set(CMAKE_SYSTEM_PROCESSOR arm64 CACHE STRING "Target operating system process")
set(CMAKE_SYSTEM_VERSION "20.0.0" CACHE STRING "Target operating system version")
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

set(OSXCROSS_TARGET_TRIPLE "${CMAKE_SYSTEM_PROCESSOR}-apple-darwin${CMAKE_SYSTEM_VERSION}")

# Find the cross-compiler tooling
find_program(CMAKE_C_COMPILER clang)
find_program(CMAKE_CXX_COMPILER clang++)
find_program(CMAKE_INSTALL_NAME_TOOL ${OSXCROSS_TARGET_TRIPLE}-install_name_tool)
find_program(CMAKE_LINKER ${OSXCROSS_TARGET_TRIPLE}-ld)

# Restrict library and header lookup to the OSX sysroot
set(CMAKE_FIND_ROOT_PATH ${CMAKE_OSX_SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_C_FLAGS_INIT "-target ${OSXCROSS_TARGET_TRIPLE}")
set(CMAKE_CXX_FLAGS_INIT "-target ${OSXCROSS_TARGET_TRIPLE}")
set(CMAKE_OBJC_FLAGS_INIT "-target ${OSXCROSS_TARGET_TRIPLE}")
set(CMAKE_OBJCXX_FLAGS_INIT "-target ${OSXCROSS_TARGET_TRIPLE}")
if(DEFINED ENV{CONDA_PREFIX})
    set(CMAKE_EXE_LINKER_FLAGS "" CACHE STRING "Executable linker flags")
    set(CMAKE_SHARED_LINKER_FLAGS "" CACHE STRING "Shared library linker flags")
    set(CMAKE_MODULE_LINKER_FLAGS "" CACHE STRING "Module linker flags")
endif()
