# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set(COMMIT_SHA 1225ed24c739f044a6c2dc12e4fb308a2aaac1cf)
set(BASE_URI https://github.com/mullvad/mullvadvpn-app-binaries/raw/${COMMIT_SHA}/${WINDOWS_TARGET_TRIPLET}/split-tunnel/)

 # Download the prebuilt binaries from Mullvad
if(${WINDOWS_TARGET_TRIPLET} STREQUAL "aarch64-pc-windows-msvc")

    file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.cat ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.cat
        EXPECTED_HASH SHA256=6B302941A93441D7522FFD9571431123A073395258D21B3890D1C839F5A2F4BD)
    file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.inf ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.inf
        EXPECTED_HASH SHA256=8A0B893A785CF07C303B55C62014B40333D946552824B91445F483412ECB23D0)
    file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.sys ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.sys
        EXPECTED_HASH SHA256=86A2485D2D9D12477F773FB91A134E77BC6B8F686CA50CAEB704878D524BB717)

elseif(${WINDOWS_TARGET_TRIPLET} STREQUAL "x86_64-pc-windows-msvc")

    file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.cat ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.cat
    EXPECTED_HASH SHA256=3630068C6BE6C832C16FF5C4C5F6B31D8940320EE6BC1D7463E1FF295F3ABD6B)
    file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.inf ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.inf
    EXPECTED_HASH SHA256=CC439CE33CA156C3DF85E17C0551BED07437EEDBDA32913A2DDC0E8B584B61D8)
    file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.sys ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.sys
    EXPECTED_HASH SHA256=08D14A289EB9A071194E30A82AFF5A936DCA7EF8827FC6986075EDDF1A6CB8FA)
else()  
    message(FATAL "Unexcpeted WINDOWS_TARGET_TRIPLET -> No Split Tunnel driver files for $WINDOWS_TARGET_TRIPLET")
endif()



# Install the prebuilt binaries
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.cat
    ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.inf
    ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.sys
    DESTINATION .
)
