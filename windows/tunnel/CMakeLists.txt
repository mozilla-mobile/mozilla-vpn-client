# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Download and extract wintun.dll and friends.
file(DOWNLOAD https://www.wintun.net/builds/wintun-0.12.zip
    ${CMAKE_CURRENT_BINARY_DIR}/.deps/wintun.zip
    EXPECTED_HASH SHA256=eba90e26686ed86595ae0a6d4d3f4f022924b1758f5148a32a91c60cc6e604df)

file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/.deps/wintun.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.deps)

## Build the tunnel DLL
add_custom_target(tunneldll ALL
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/tunnel.dll ${CMAKE_CURRENT_BINARY_DIR}/tunnel.h
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${CMAKE_COMMAND} -E env 
                GOCACHE=${CMAKE_BINARY_DIR}/go-cache
                GOOS=windows CGO_ENABLED=1
                CC=clang-cl
                CGO_CFLAGS="-O3 -Wall -Wno-unused-function -Wno-switch -std=gnu11 -DWINVER=0x0601"
                CGO_LDFLAGS="-Wl,--dynamicbase -Wl,--nxcompat -Wl,--export-all-symbols -Wl,--high-entropy-va"
            go build -buildmode c-shared -ldflags="-w -s" -trimpath -v -o "${CMAKE_CURRENT_BINARY_DIR}/tunnel.dll"
)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_BINARY_DIR}/go-cache)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/tunnel.dll
    ${CMAKE_CURRENT_BINARY_DIR}/.deps/wintun/bin/${CMAKE_SYSTEM_PROCESSOR}/wintun.dll
    DESTINATION .
)
