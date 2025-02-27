# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

# This File Sets WINDOWS_TARGET_TRIPLET
# Based on the Current provided compiler


# Find out what we are trying to compile for
# In MSVC land for each output arch, we get a specific compiler executable. 
# so you have one for (host: x64)->(target:x64) and another for
# (host:x64)->(target:arm64). So we need to figure out what the target is
# based on which compiler was invoked.
# 
# Also let's save that in the way rust expects tartgets
# so we can just forward this to cargo. 

if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC/CL specific logic
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} /?
        OUTPUT_VARIABLE COMPILER_OUTPUT
        ERROR_QUIET
    )
    string(FIND "${COMPILER_OUTPUT}" "x64" FOUND_X64)
    string(FIND "${COMPILER_OUTPUT}" "ARM64" FOUND_ARM64)
    if(FOUND_X64 GREATER -1)
        set(WINDOWS_TARGET_TRIPLET "x86_64-pc-windows-msvc")
    elseif(FOUND_ARM64 GREATER -1)
        set(WINDOWS_TARGET_TRIPLET "aarch64-pc-windows-msvc")
    else()
        set(WINDOWS_TARGET_TRIPLET "i686-pc-windows-msvc")
    endif()

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    # Use the `-dumpmachine` option available in GCC/Clang
    execute_process(
        COMMAND ${CMAKE_CXX_COMPILER} -dumpmachine
        OUTPUT_VARIABLE COMPILER_TARGET
        OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    set(WINDOWS_TARGET_TRIPLET "${COMPILER_TARGET}")

else()
    message(WARNING "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")
    set(WINDOWS_TARGET_TRIPLET "unknown")
endif()

# Output the Rust target triplet
message("Detected Rust target triplet: ${WINDOWS_TARGET_TRIPLET}")
