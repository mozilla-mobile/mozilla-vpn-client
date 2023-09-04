# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    # We're using clang in msvc mode so we can only use 
    # msvc style compile flags
else()
    # These compile flags are only supported on cl.exe 
    # they do nothing on clang. 
    add_compile_options(/MP /Zc:preprocessor)
endif()


set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
