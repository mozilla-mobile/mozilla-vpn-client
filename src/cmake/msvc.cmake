# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang$")
    # We're using clang in msvc mode so we can only use 
    # msvc style compile flags
    # We need to disable /Manifest tool as clang's implementation
    # of mt.exe (llvm-mt) is not a complete drop in soloution. 
    set(CMAKE_EXE_LINKER_FLAGS    "${CMAKE_EXE_LINKER_FLAGS} /MANIFEST:NO")
    set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} /MANIFEST:NO")
else()
    # /MP -> Compile Files Paraell
    # /Zc:preprocessor -> Enable Modern Macros, needed for settingsholder
    set(CMAKE_CXX_FLAGS  "/MP /Zc:preprocessor /wd5105")
    # Enable "edit and continue" when using msvc + debug build
    set(CMAKE_CXX_FLAGS_DEBUG  "/MTd /ZI /Ob0 /Od /RTC1")
    set(CMAKE_CXX_FLAGS_DEBUG  "/MTd /ZI /Ob0 /Od /RTC1")
    set(CMAKE_MODULE_LINKER_FLAGS_DEBUG  "/debug /INCREMENTAL /LTCG:OFF")
    set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/debug /INCREMENTAL /LTCG:OFF")
    set(CMAKE_EXE_LINKER_FLAGS_DEBUG  "/debug /INCREMENTAL /LTCG:OFF")
    
endif()

