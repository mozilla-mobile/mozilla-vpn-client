# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Download and extract Wix and friends.


set(WIREGUARD_FOLDER $ENV{WIREGUARD_FOLDER} CACHE STRING "Path to Extracted WireGuardNT Folder")

if(NOT WIREGUARD_FOLDER)
    file(DOWNLOAD https://download.wireguard.com/wireguard-nt/wireguard-nt-0.10.1.zip
        ${CMAKE_CURRENT_BINARY_DIR}/.deps/wg-nt.zip
        EXPECTED_HASH SHA256=772c0b1463d8d2212716f43f06f4594d880dea4f735165bd68e388fc41b81605)

    file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/.deps/wg-nt.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/.deps)

    set(WIREGUARD_FOLDER ${CMAKE_CURRENT_BINARY_DIR}/.deps/wireguard-nt) 
endif()



include_directories(${WIREGUARD_FOLDER}/include)

install(FILES
    ${WIREGUARD_FOLDER}/bin/${CMAKE_SYSTEM_PROCESSOR}/wireguard.dll
    DESTINATION .
)

target_include_directories(mozillavpn PUBLIC ${WIREGUARD_FOLDER}/include)
target_include_directories(shared-sources INTERFACE ${WIREGUARD_FOLDER}/include)
