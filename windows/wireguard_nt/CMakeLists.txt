# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.

## Download and extract Wix and friends.
file(DOWNLOAD https://download.wireguard.com/wireguard-nt/wireguard-nt-0.10.1.zip
    ${CMAKE_CURRENT_BINARY_DIR}/.deps/wg-nt.zip
    EXPECTED_HASH SHA256=772c0b1463d8d2212716f43f06f4594d880dea4f735165bd68e388fc41b81605)

file(ARCHIVE_EXTRACT INPUT ${CMAKE_CURRENT_BINARY_DIR}/.deps/wg-nt.zip DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_BINARY_DIR}/wireguard-nt/include)

install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/wireguard-nt/bin/{CMAKE_SYSTEM_PROCESSOR}/wireguard.dll
    DESTINATION .
)

target_include_directories(mozillavpn PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/wireguard-nt/include)
target_include_directories(shared-sources INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/wireguard-nt/include)
set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES ${CMAKE_CURRENT_BINARY_DIR}/staging)
