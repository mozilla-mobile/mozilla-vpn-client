# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

set(WIREGUARD_FOLDER $ENV{WIREGUARD_FOLDER} CACHE STRING "Path to Extracted WireGuardNT Folder")

if(NOT WIREGUARD_FOLDER)
    file(DOWNLOAD https://download.wireguard.com/wireguard-nt/wireguard-nt-1.1.zip
        ${CMAKE_CURRENT_BINARY_DIR}/.deps/wg-nt.zip
        EXPECTED_HASH SHA256=dceb30a9bc4be48cce0f74160fc88a585a2c2627366e8f846fc6658f9038dace)

    file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/.deps/wg-nt.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.deps)

    set(CM_WIREGUARD_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/.deps/wireguard-nt) 
else()
    cmake_path(CONVERT ${WIREGUARD_FOLDER} TO_CMAKE_PATH_LIST CM_WIREGUARD_FOLDER)
endif()



include_directories(${CM_WIREGUARD_FOLDER}/include)

install(FILES
    ${CM_WIREGUARD_FOLDER}/bin/${CMAKE_SYSTEM_PROCESSOR}/wireguard.dll
    DESTINATION .
)

target_include_directories(mozillavpn PUBLIC ${CM_WIREGUARD_FOLDER}/include)
target_include_directories(shared-sources INTERFACE ${CM_WIREGUARD_FOLDER}/include)
