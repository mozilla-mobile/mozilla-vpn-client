# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

add_compile_options($<$<COMPILE_LANGUAGE:CXX>:-fvisibility=hidden>)

# Provide fallbacks to LLVM tooling if the MSVC versions aren't detected.
if(WIN32)
    if(NOT CMAKE_MT)
        find_program(CMAKE_MT NAMES llvm-mt REQUIRED DOC "LLVM Manifest Tool")
    endif()
endif()
