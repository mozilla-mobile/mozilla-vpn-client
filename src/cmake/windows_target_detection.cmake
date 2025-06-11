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
# We can't rely on CMAKE_HOSTARCH schenenigans given cmake might 
# run under rosetta as an emulated x64 binary and doesn't even know itself. 
# 
# Also let's save that in the way rust expects tartgets
# so we can just forward this to cargo. 

if(NOT DEFINED WINDOWS_TARGET_TRIPLET)
    set(WINDOWS_TARGET_TRIPLET "x86_64-pc-windows-msvc")
    message(STATUS "WINDOWS_TARGET_TRIPLET not defined, defaulting to ${WINDOWS_TARGET_TRIPLET}")
else()
    message(STATUS "WINDOWS_TARGET_TRIPLET is already defined as ${WINDOWS_TARGET_TRIPLET}")
endif()

# Output the Rust target triplet
message("Detected Rust target triplet: ${WINDOWS_TARGET_TRIPLET}")
