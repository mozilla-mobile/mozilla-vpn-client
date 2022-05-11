# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
set(COMMIT_SHA 375cfc52255ec3beb15cc0713988ea995cb7582e)
set(BASE_URI https://github.com/mullvad/mullvadvpn-app-binaries/raw/${COMMIT_SHA}/x86_64-pc-windows-msvc/split-tunnel/win10/)

# Download the prebuilt binaries from Mullvad
file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.cat ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.cat
    EXPECTED_HASH SHA256=385C90BE86B6C934344C6D2647887182B41D7E37803DC5EABE7E0805F6210317)
file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.inf ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.inf
    EXPECTED_HASH SHA256=A9500015B0C93C96D6859E942CF76B6B637653F46ADDEE11BB07AF0C1EA6E879)
file(DOWNLOAD ${BASE_URI}/mullvad-split-tunnel.sys ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.sys
    EXPECTED_HASH SHA256=8BDC9FC6AA1203E444A2D6E3A564CF8DD8F7EC9DB850F0095B9D33E16B96AFF5)
file(DOWNLOAD ${BASE_URI}/WdfCoinstaller01011.dll ${CMAKE_CURRENT_BINARY_DIR}/WdfCoinstaller01011.dll
    EXPECTED_HASH SHA256=B2CE2291D10834E132EC1A349B726822E8C943B47AF968E9DE592ED7AE1CFA36)

# Install the prebuilt binaries
install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.cat
    ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.inf
    ${CMAKE_CURRENT_BINARY_DIR}/mullvad-split-tunnel.sys
    ${CMAKE_CURRENT_BINARY_DIR}/WdfCoinstaller01011.dll
    DESTINATION .
)
